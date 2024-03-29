/************************************************************************
COPYRIGHT (C) SGS-THOMSON Microelectronics 2007

Source file name : codec_mme_video_H264.cpp
Author :           Nick

Implementation of the H264 video codec class for player 2.

This is a slightly (read humungously) more complicated beast than
the Mpeg2 creature of the same ilk. The H264 codec takes buffers
containing slices, passes/concatenates them through the pre-processor
(on to an intermediate process), and then on to the mainline ancestor
classes as complete field/frame entities. As such this outer layer must
superclass almost all of the entrypoints to handle calls relating to
structures/buffers that may not yet have made it to the ancestor class.

Also since this outer layer is concatenating slices etc., it actually
tells the ancestor class that the ancestor class need not handle slice
level decoding.


Date        Modification                                    Name
----        ------------                                    --------
16-Aug-07   Created                                         Nick

************************************************************************/

// /////////////////////////////////////////////////////////////////////
//
//      Include any component headers

#include "codec_mme_video_h264.h"
#include "h264.h"
#include "ring_generic.h"

#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
#include "manifestor_base.h"
#endif ///CONFIG_SH_QBOXHD_1_0

// /////////////////////////////////////////////////////////////////////////
//
// Locally defined constants
//

#define MAX_EVENT_WAIT          100             // ms

// /////////////////////////////////////////////////////////////////////////
//
// Locally defined structures
//

typedef struct H264CodecStreamParameterContext_s
{
    CodecBaseStreamParameterContext_t   BaseContext;

    H264_SetGlobalParam_t               StreamParameters;
} H264CodecStreamParameterContext_t;

#define BUFFER_H264_CODEC_STREAM_PARAMETER_CONTEXT             "H264CodecStreamParameterContext"
#define BUFFER_H264_CODEC_STREAM_PARAMETER_CONTEXT_TYPE        {BUFFER_H264_CODEC_STREAM_PARAMETER_CONTEXT, BufferDataTypeBase, AllocateFromOSMemory, 32, 0, true, true, sizeof(H264CodecStreamParameterContext_t)}

static BufferDataDescriptor_t            H264CodecStreamParameterContextDescriptor = BUFFER_H264_CODEC_STREAM_PARAMETER_CONTEXT_TYPE;

// --------

typedef struct H264CodecDecodeContext_s
{
    CodecBaseDecodeContext_t            BaseContext;

    H264_TransformParam_fmw_t           DecodeParameters;
    H264_CommandStatus_fmw_t            DecodeStatus;
} H264CodecDecodeContext_t;

#define BUFFER_H264_CODEC_DECODE_CONTEXT       "H264CodecDecodeContext"
#define BUFFER_H264_CODEC_DECODE_CONTEXT_TYPE  {BUFFER_H264_CODEC_DECODE_CONTEXT, BufferDataTypeBase, AllocateFromOSMemory, 32, 0, true, true, sizeof(H264CodecDecodeContext_t)}

static BufferDataDescriptor_t            H264CodecDecodeContextDescriptor = BUFFER_H264_CODEC_DECODE_CONTEXT_TYPE;

// --------

#define BUFFER_H264_PRE_PROCESSOR_BUFFER        "H264PreProcessorBuffer"
#define BUFFER_H264_PRE_PROCESSOR_BUFFER_TYPE	{BUFFER_H264_PRE_PROCESSOR_BUFFER, BufferDataTypeBase, AllocateFromNamedDeviceMemory, 32, 0, true, true, H264_PP_BUFFER_SIZE}

static BufferDataDescriptor_t           H264PreProcessorBufferDescriptor = BUFFER_H264_PRE_PROCESSOR_BUFFER_TYPE;

// --------

#define BUFFER_H264_MACROBLOCK_STRUCTURE_BUFFER "H264MacroBlockStructureBuffer"
#define BUFFER_H264_MACROBLOCK_STRUCTURE_BUFFER_TYPE	{BUFFER_H264_MACROBLOCK_STRUCTURE_BUFFER, BufferDataTypeBase, AllocateFromSuppliedBlock, 256, 256, false, false, NOT_SPECIFIED}

static BufferDataDescriptor_t           H264MacroBlockStructureBufferDescriptor = BUFFER_H264_MACROBLOCK_STRUCTURE_BUFFER_TYPE;

// /////////////////////////////////////////////////////////////////////////
//
//      Cosntructor function, fills in the codec specific parameter values
//

Codec_MmeVideoH264_c::Codec_MmeVideoH264_c( void )
{
OS_Status_t               OSStatus;
OS_Thread_t               Thread;

//

    Configuration.CodecName                             = "H264 video";

    Configuration.CodedFrameCount                       = H264_CODED_FRAME_COUNT;
    Configuration.CodedMemorySize                       = H264_FRAME_MEMORY_SIZE;
    Configuration.MaximumCodedFrameSize                 = H264_MAXIMUM_FRAME_SIZE;

    Configuration.DecodeOutputFormat                    = FormatVideo420_MacroBlock;

    Configuration.StreamParameterContextCount           = 4;                    // H264 tests use stream param change per frame
    Configuration.StreamParameterContextDescriptor      = &H264CodecStreamParameterContextDescriptor;

    Configuration.DecodeContextCount                    = 4;
    Configuration.DecodeContextDescriptor               = &H264CodecDecodeContextDescriptor;

    Configuration.MaxDecodeIndicesPerBuffer             = 2;
    Configuration.SliceDecodePermitted                  = false;

    Configuration.TransformName[0]                      = H264_MME_TRANSFORMER_NAME "0";
    Configuration.TransformName[1]                      = H264_MME_TRANSFORMER_NAME "1";
    Configuration.AvailableTransformers                 = 2;

    Configuration.SizeOfTransformCapabilityStructure    = sizeof(H264_TransformerCapability_fmw_t);
    Configuration.TransformCapabilityStructurePointer   = (void *)(&H264TransformCapability);

    //
    // The video firmware violates the MME spec. and passes data buffer addresses
    // as parametric information. For this reason it requires physical addresses
    // to be used.
    //

    Configuration.AddressingMode                        = PhysicalAddress;

    //
    // Since we pre-process the data, we shrink the coded data buffers
    // before enterring the generic code. as a consequence we do
    // not require the generic code to shrink buffers on our behalf,
    // and we do not want the generic code to find the coded data buffer
    // for us.
    //

    Configuration.ShrinkCodedDataBuffersAfterDecode     = false;
    Configuration.IgnoreFindCodedDataBuffer		= true;

    //
    // My trick mode parameters
    //

    Configuration.TrickModeParameters.SubstandardDecodeSupported        = true;
    Configuration.TrickModeParameters.MaximumNormalDecodeRate           = Rational_t(3,2);
    Configuration.TrickModeParameters.MaximumSubstandardDecodeRate      = 2;

    Configuration.TrickModeParameters.DefaultGroupSize                  = 12;
    Configuration.TrickModeParameters.DefaultGroupReferenceFrameCount   = 4;

    Configuration.TrickModeParameters.AutoAdjustDecodeRates		= true;
    Configuration.TrickModeParameters.PixelsAtNormalRate		= 1920 * 1080;
    Configuration.TrickModeParameters.FrameRateAtNormalRate		= 30;
    Configuration.TrickModeParameters.BaseNormalDecodeRate		= Configuration.TrickModeParameters.MaximumNormalDecodeRate;
    Configuration.TrickModeParameters.BaseSubstandardDecodeRate		= Configuration.TrickModeParameters.MaximumSubstandardDecodeRate;

    //
    // Initailize variables before performing reset
    //

    FramesInPreprocessorChainRing       = NULL;
    MacroBlockStructurePool             = NULL;
    PreProcessorBufferPool              = NULL;
    PreProcessorDevice                  = H264_PP_INVALID_DEVICE;
    MacroBlockMemoryDevice		= ALLOCATOR_INVALID_DEVICE;

    Terminating                         = false;
    ProcessRunningCount                 = 0;
    OS_InitializeEvent( &StartStopEvent );

    Reset();

    //
    // Create the lock
    //

    OS_InitializeMutex( &H264Lock );

#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
    OS_InitializeMutex(&HALT_Lock);
    HALT_Flag = false;
    HALTED_Flag = false;
#endif ///CONFIG_SH_QBOXHD_1_0

    //
    // Create the ring used to inform the intermediate process of a datum on the way
    //

    FramesInPreprocessorChainRing       = new class RingGeneric_c(H264_CODED_FRAME_COUNT);
    if( (FramesInPreprocessorChainRing == NULL) ||
	(FramesInPreprocessorChainRing->InitializationStatus != RingNoError) )
    {
	report( severity_error, "Codec_MmeVideoH264_c::Codec_MmeVideoH264_c - Failed to create ring to hold frames in pre-processor chain\n" );
	InitializationStatus    = CodecError;
	return;
    }

    //
    // Create the ring used to hold reference frame indices in the intermediate process
    //

    ReleaseReferenceFrameStash          = new class RingGeneric_c(CODEC_MAX_DECODE_BUFFERS + 1);
    if( (ReleaseReferenceFrameStash == NULL) ||
	(ReleaseReferenceFrameStash->InitializationStatus != RingNoError) )
    {
	report( severity_error, "Codec_MmeVideoH264_c::Codec_MmeVideoH264_c - Failed to create ring to hold reference frame indices in pre-processor chain\n" );
	InitializationStatus    = CodecError;
	return;
    }

    //
    // Create the intermediate process
    //

    OS_ResetEvent( &StartStopEvent );

    OSStatus    = OS_CreateThread( &Thread, Codec_MmeVideoH264_IntermediateProcess, this, "H264 Codec Int", (OS_MID_PRIORITY+9) );
    if( OSStatus != OS_NO_ERROR )
    {
	report( severity_error, "Codec_MmeVideoH264_c::Codec_MmeVideoH264_c - Failed to create intermediate processe.\n" );
	Terminating     = true;
	OS_SleepMilliSeconds( MAX_EVENT_WAIT );
	InitializationStatus    = CodecError;
	return;
    }

    OS_WaitForEvent( &StartStopEvent, MAX_EVENT_WAIT );

    if( ProcessRunningCount != 1 )
    {
	report( severity_error, "Codec_MmeVideoH264_c::Codec_MmeVideoH264_c - Intermediate processe failed to run.\n" );
	Terminating     = true;
	OS_SleepMilliSeconds( MAX_EVENT_WAIT );
	InitializationStatus    = CodecError;
	return;
    }
}


// /////////////////////////////////////////////////////////////////////////
//
//      Destructor function, ensures a full halt and reset
//      are executed for all levels of the class.
//

Codec_MmeVideoH264_c::~Codec_MmeVideoH264_c( void )
{
    Halt();
    Reset();

    //
    // Shutdown the intermediate process
    //

    OS_ResetEvent( &StartStopEvent );
    Terminating         = true;
    if( ProcessRunningCount != 0 )
    {
	OS_WaitForEvent( &StartStopEvent, MAX_EVENT_WAIT );

	if(  ProcessRunningCount != 0 )
	    report( severity_error, "Codec_MmeVideoH264_c::~Codec_MmeVideoH264_c - Intermediate processe failed to terminate.\n" );
    }

    //
    // Delete the frames in pre-processor chain ring
    //

    if( FramesInPreprocessorChainRing != NULL )
    {
	delete FramesInPreprocessorChainRing;
	FramesInPreprocessorChainRing   = NULL;
    }

    if( ReleaseReferenceFrameStash != NULL )
    {
	delete ReleaseReferenceFrameStash;
	ReleaseReferenceFrameStash      = NULL;
    }

    //
    // Destroy the locking mutex
    //

    OS_TerminateEvent( &StartStopEvent );
    OS_TerminateMutex( &H264Lock );

#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
    OS_TerminateMutex( &HALT_Lock );
#endif ///CONFIG_SH_QBOXHD_1_0
}


// /////////////////////////////////////////////////////////////////////////
//
//      Halt function
//

CodecStatus_t   Codec_MmeVideoH264_c::Halt(             void )
{
#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
    int status;                                                                                              //Entire function modified by Duolabs

    while(!HALTED_Flag)
    {
        status = OS_LockMutex_trylock(&HALT_Lock);
        if (status)
        {
            //report( severity_error, "Codec_MmeVideoH264_c::Halt H264  Tryed to HALT++++++++++++++++++++++++++++++++++++++++++\n");
            HALT_Flag = true;
            OS_SleepMilliSeconds( 100 );
            continue;
        }
        break;
    }

    HALTED_Flag = true;
    //report( severity_error, "Codec_MmeVideoH264_c::Halt H264  HALTED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
#endif ///CONFIG_SH_QBOXHD_1_0

    return Codec_MmeVideo_c::Halt();
}


// /////////////////////////////////////////////////////////////////////////
//
//      Reset function
//

CodecStatus_t   Codec_MmeVideoH264_c::Reset(            void )

{
    //
    // Free buffer pools
    //

    if( PreProcessorBufferPool != NULL )
    {
	BufferManager->DestroyPool( PreProcessorBufferPool );
	PreProcessorBufferPool  = NULL;
    }

    if( MacroBlockStructurePool != NULL )
    {
	BufferManager->DestroyPool( MacroBlockStructurePool );
	MacroBlockStructurePool = NULL;
    }

    if( MacroBlockMemoryDevice != ALLOCATOR_INVALID_DEVICE )
    {
	AllocatorClose( MacroBlockMemoryDevice );
	MacroBlockMemoryDevice	= ALLOCATOR_INVALID_DEVICE;
    }

    //
    // Make sure the pre-processor device is closed (note this may be done in halt,
    // but we check again, since you can enter reset without halt during a failed
    // start up).
    //

    if( PreProcessorDevice != H264_PP_INVALID_DEVICE )
    {
	H264ppClose( PreProcessorDevice );
	PreProcessorDevice      = H264_PP_INVALID_DEVICE;
    }

    //
    // Re-initialize variables
    //

    LastSeenDecodeIndex                 = INVALID_INDEX;
    AccumulatingFrame                   = INVALID_INDEX;
    OutstandingSlotAllocationRequest	= INVALID_INDEX;
    DiscardFramesInPreprocessorChain    = false;
    memset( FramesInPreprocessorChain, 0x00, H264_CODED_FRAME_COUNT * sizeof(FramesInPreprocessorChain_t) );
    memset( RecordedHostData, 0x00, CODEC_MAX_DECODE_BUFFERS * sizeof(H264_HostData_t) );
    memset( ReferenceFrameSlotUsed, 0x00, H264_MAX_REFERENCE_FRAMES * sizeof(bool) );

//

    return Codec_MmeVideo_c::Reset();
}


// /////////////////////////////////////////////////////////////////////////
//
//      Register the output buffer ring, we take this opertunity to
//      create the buffer pools for use in the pre-processor, and the
//      macroblock structure buffers
//

CodecStatus_t   Codec_MmeVideoH264_c::RegisterOutputBufferRing(         Ring_t          Ring )
{
CodecStatus_t		Status;
allocator_status_t	AStatus;
h264pp_status_t		PPStatus;

    //
    // Let the ancestor classes setup the framework
    //

    Status      = Codec_MmeVideo_c::RegisterOutputBufferRing( Ring );
    if( Status != CodecNoError )
	return Status;

    //
    // Create the pre-processor buffer pool
    //

    Status      = BufferManager->FindBufferDataType( BUFFER_H264_PRE_PROCESSOR_BUFFER, &PreProcessorBufferType );
    if( Status != BufferNoError )
    {
	Status  = BufferManager->CreateBufferDataType( &H264PreProcessorBufferDescriptor, &PreProcessorBufferType );
	if( Status != BufferNoError )
	{
	    report( severity_error, "Codec_MmeVideoH264_c::RegisterOutputBufferRing - Failed to create the %s buffer type (%08x).\n", BUFFER_H264_PRE_PROCESSOR_BUFFER, Status );
	    SetComponentState(ComponentInError);
	    return Status;
	}
    }

//

    Status	= BufferManager->CreatePool( &PreProcessorBufferPool, PreProcessorBufferType, Configuration.DecodeContextCount, UNSPECIFIED_SIZE,
					     NULL, NULL, Configuration.AncillaryMemoryPartitionName );
    if( Status != BufferNoError )
    {
	report( severity_error, "Codec_MmeVideoH264_c::RegisterOutputBufferRing - Failed to create pre-processor buffer pool (%08x).\n", Status );
	SetComponentState(ComponentInError);
	return Status;
    }

    //
    // Obtain the memory for the macroblock structure pool
    //

    AStatus = PartitionAllocatorOpen( &MacroBlockMemoryDevice, Configuration.AncillaryMemoryPartitionName, H264_MACROBLOCK_STRUCTURE_MEMORY, true );
    if( AStatus != allocator_ok )
    {
	report( severity_error, "Codec_MmeVideoH264_c::RegisterOutputBufferRing - Failed to allocate memory for macroblock structure buffers\n" );
	return PlayerInsufficientMemory;
    }

    MacroBlockMemory[CachedAddress]	= AllocatorUserAddress( MacroBlockMemoryDevice );
    MacroBlockMemory[UnCachedAddress]	= AllocatorUncachedUserAddress( MacroBlockMemoryDevice );
    MacroBlockMemory[PhysicalAddress]	= AllocatorPhysicalAddress( MacroBlockMemoryDevice );

    //
    // Create the macroblock structure buffer pool
    // NOTE ref frames + 4, ref frames + 1 copes with 1
    // for each rerefernce frame plus the one being decoded.
    // We then add 3 because these are kept while a decode buffer
    // goes to the manifestor and back.
    //

    Status      = BufferManager->FindBufferDataType( BUFFER_H264_MACROBLOCK_STRUCTURE_BUFFER, &MacroBlockStructureType );
    if( Status != BufferNoError )
    {
	Status  = BufferManager->CreateBufferDataType( &H264MacroBlockStructureBufferDescriptor, &MacroBlockStructureType );
	if( Status != BufferNoError )
	{
	    report( severity_error, "Codec_MmeVideoH264_c::RegisterOutputBufferRing - Failed to create the %s buffer type (%08x).\n", BUFFER_H264_MACROBLOCK_STRUCTURE_BUFFER, Status );
	    SetComponentState(ComponentInError);
	    return Status;
	}
    }

//

    Status	= BufferManager->CreatePool( &MacroBlockStructurePool, MacroBlockStructureType, (H264_MAX_REFERENCE_FRAMES+4), H264_MACROBLOCK_STRUCTURE_MEMORY, MacroBlockMemory );
    if( Status != BufferNoError )
    {
	report( severity_error, "Codec_MmeVideoH264_c::RegisterOutputBufferRing - Failed to create macro block structure buffer pool (%08x).\n", Status );
	SetComponentState(ComponentInError);
	return Status;
    }

//

    PPStatus    = H264ppOpen( &PreProcessorDevice );
    if( PPStatus != h264pp_ok )
    {
	report( severity_error, "Codec_MmeVideoH264_c::RegisterOutputBufferRing - Failed to open the pre-processor device.\n" );
	SetComponentState(ComponentInError);
	return Status;
    }

//

    return CodecNoError;
}


// /////////////////////////////////////////////////////////////////////////
//
//      Output partial decode buffers - not entirely sure what to do here,
//      it may well be necessary to fire off the pre-processing of any
//      accumulated slices.
//

CodecStatus_t   Codec_MmeVideoH264_c::OutputPartialDecodeBuffers( void )
{
unsigned int    i;

//

    OS_LockMutex( &H264Lock );

    //
    // If we have any accumulated data this call implies it is a complete frame,
    // in order to maintain ordering we must fire this off now.
    //

    FireOffPreProcessing();

//

    for( i=0; i<H264_CODED_FRAME_COUNT; i++ )
	if( !FramesInPreprocessorChain[i].Used )
	{
	    memset( &FramesInPreprocessorChain[i], 0x00, sizeof(FramesInPreprocessorChain_t) );
	    FramesInPreprocessorChain[i].Used                                   = true;
	    FramesInPreprocessorChain[i].Action                                 = ActionCallOutputPartialDecodeBuffers;
	    FramesInPreprocessorChain[i].EmptyReleaseReferenceFrameStash        = true;

	    FramesInPreprocessorChainRing->Insert( i );
	    OS_UnLockMutex( &H264Lock );
	    return CodecNoError;
	}

    OS_UnLockMutex( &H264Lock );
    report( severity_error, "Codec_MmeVideoH264_c::OutputPartialDecodeBuffers - Failed to find an entry in the \"FramesInPreprocessorChain\" list.\n" );
    return CodecError;
}


// /////////////////////////////////////////////////////////////////////////
//
//      Discard queued decodes, this will include any between here
//      and the preprocessor.
//

CodecStatus_t   Codec_MmeVideoH264_c::DiscardQueuedDecodes( void )
{
unsigned int    i;

//

    OS_LockMutex( &H264Lock );
    if( (AccumulatingFrame   != INVALID_INDEX) ||
	(LastSeenDecodeIndex != INVALID_INDEX) )
    {
	report( severity_info, "Codec_MmeVideoH264_c::DiscardQueuedDecodes - Discard while still accumulating data, junking frame.\n" );
	FramesInPreprocessorChain[AccumulatingFrame].Used       = false;
	FramesInPreprocessorChain[AccumulatingFrame].CodedBuffer->DecrementReferenceCount( IdentifierH264Intermediate );

	AccumulatingFrame       = INVALID_INDEX;
	LastSeenDecodeIndex     = INVALID_INDEX;
    }

//

    for( i=0; i<H264_CODED_FRAME_COUNT; i++ )
	if( !FramesInPreprocessorChain[i].Used )
	{
	    DiscardFramesInPreprocessorChain            = true;

	    memset( &FramesInPreprocessorChain[i], 0x00, sizeof(FramesInPreprocessorChain_t) );
	    FramesInPreprocessorChain[i].Used                                   = true;
	    FramesInPreprocessorChain[i].Action                                 = ActionCallDiscardQueuedDecodes;
	    FramesInPreprocessorChain[i].EmptyReleaseReferenceFrameStash        = true;

	    FramesInPreprocessorChainRing->Insert( i );
	    OS_UnLockMutex( &H264Lock );
	    return CodecNoError;
	}

    OS_UnLockMutex( &H264Lock );
    report( severity_error, "Codec_MmeVideoH264_c::DiscardQueuedDecodes - Failed to find an entry in the \"FramesInPreprocessorChain\" list.\n" );
    return CodecError;
}


// /////////////////////////////////////////////////////////////////////////
//
//      Release reference frame, this must be capable of releasing a
//      reference frame that has not yet exited the pre-processor.
//

CodecStatus_t   Codec_MmeVideoH264_c::ReleaseReferenceFrame(    unsigned int              ReferenceFrameDecodeIndex )
{
unsigned int    i;

//

    OS_LockMutex( &H264Lock );

    for( i=0; i<H264_CODED_FRAME_COUNT; i++ )
	if( !FramesInPreprocessorChain[i].Used )
	{
	    memset( &FramesInPreprocessorChain[i], 0x00, sizeof(FramesInPreprocessorChain_t) );
	    FramesInPreprocessorChain[i].Used                   = true;
	    FramesInPreprocessorChain[i].DecodeFrameIndex       = ReferenceFrameDecodeIndex;
	    FramesInPreprocessorChain[i].Action                 = ( AccumulatingFrame != INVALID_INDEX ) ?
									ActionStashReleaseReferenceFrame :
									ActionCallReleaseReferenceFrame;

	    FramesInPreprocessorChainRing->Insert( i );
	    OS_UnLockMutex( &H264Lock );
	    return CodecNoError;
	}

    OS_UnLockMutex( &H264Lock );
    report( severity_error, "Codec_MmeVideoH264_c::ReleaseReferenceFrame - Failed to find an entry in the \"FramesInPreprocessorChain\" list.\n" );
    return CodecError;
}


// /////////////////////////////////////////////////////////////////////////
//
//      Check reference frame list needs to account for those currently
//      in the pre-processor chain.
//

CodecStatus_t   Codec_MmeVideoH264_c::CheckReferenceFrameList(
							unsigned int              NumberOfReferenceFrameLists,
							ReferenceFrameList_t      ReferenceFrameList[] )
{
unsigned int            i, j, k;
ReferenceFrameList_t    LocalReferenceFrameList[H264_NUM_REF_FRAME_LISTS];

    //
    // Check we can cope
    //

    if( NumberOfReferenceFrameLists > H264_NUM_REF_FRAME_LISTS )
    {
	report( severity_error, "Codec_MmeVideoH264_c::CheckReferenceFrameList - H264 has only %d reference frame lists, requested to check %d\n", H264_NUM_REF_FRAME_LISTS, NumberOfReferenceFrameLists );
	return CodecUnknownFrame;
    }

    //
    // Construct local lists consisting of those elements we do not know about
    //

    for( i=0; i<NumberOfReferenceFrameLists; i++ )
    {
	LocalReferenceFrameList[i].EntryCount   = 0;
	for( j=0; j<ReferenceFrameList[i].EntryCount; j++ )
	{
	    for( k=0; k<H264_CODED_FRAME_COUNT; k++ )
		if( FramesInPreprocessorChain[k].Used &&
		    (FramesInPreprocessorChain[k].DecodeFrameIndex == ReferenceFrameList[i].EntryIndicies[j]) )
		    break;

	    if( k == H264_CODED_FRAME_COUNT )
		LocalReferenceFrameList[i].EntryIndicies[LocalReferenceFrameList[i].EntryCount++]       = ReferenceFrameList[i].EntryIndicies[j];
	}
    }

    return Codec_MmeVideo_c::CheckReferenceFrameList( NumberOfReferenceFrameLists, LocalReferenceFrameList );
}


// /////////////////////////////////////////////////////////////////////////
//
//      Input, must feed the data to the preprocessor chain.
//      This function needs to replicate a considerable amount of the
//      ancestor classes, because this function is operating
//      in a different process to the ancestors, and because the
//      ancestor data, and the data used here, will be in existence
//      at the same time.
//

CodecStatus_t   Codec_MmeVideoH264_c::Input(            Buffer_t                  CodedBuffer )
{
unsigned int              i;
PlayerStatus_t            Status;
h264pp_status_t           PPStatus;
unsigned int              CodedDataLength;
unsigned char            *CodedData;
ParsedFrameParameters_t  *ParsedFrameParameters;
ParsedVideoParameters_t  *ParsedVideoParameters;
Buffer_t                  PreProcessorBuffer;
H264FrameParameters_t    *FrameParameters;

    //
    // First extract the useful pointers from the buffer all held locally
    //

    Status      = CodedBuffer->ObtainDataReference( NULL, &CodedDataLength, (void **)(&CodedData) );
    if( Status != PlayerNoError )
    {
	report( severity_error, "Codec_MmeVideoH264_c::Input - Unable to obtain data reference.\n" );
	return Status;
    }

//

    Status      = CodedBuffer->ObtainMetaDataReference( Player->MetaDataParsedFrameParametersType, (void **)(&ParsedFrameParameters) );
    if( Status != PlayerNoError )
    {
	report( severity_error, "Codec_MmeVideoH264_c::Input - Unable to obtain the meta data \"ParsedFrameParameters\".\n" );
	return Status;
    }

//

    Status      = CodedBuffer->ObtainMetaDataReference( Player->MetaDataParsedVideoParametersType, (void **)(&ParsedVideoParameters) );
    if( Status != PlayerNoError )
    {
	report( severity_error, "Codec_MmeVideoH264_c::Input - Unable to obtain the meta data \"ParsedVideoParameters\".\n" );
	return Status;
    }

    //
    // If this does not contain any slice data, then we simply wish
    // to slot it into place for the intermediate process.
    //

    OS_LockMutex( &H264Lock );

    if( !ParsedFrameParameters->NewFrameParameters ||
	(ParsedFrameParameters->DecodeFrameIndex == INVALID_INDEX) )
    {
	//
	// If we have any accumulated data this call implies it is a complete frame,
	// in order to maintain ordering we must fire this off now. However, since we
	// cannot guarantee that the frame is complete we mark it as speculative, so
 	// if the preprocessing fails the frame can be discarded.
	//

	FireOffPreProcessing( true );

//

	for( i=0; i<H264_CODED_FRAME_COUNT; i++ )
	    if( !FramesInPreprocessorChain[i].Used )
	    {
		memset( &FramesInPreprocessorChain[i], 0x00, sizeof(FramesInPreprocessorChain_t) );
		FramesInPreprocessorChain[i].Used                               = true;
		FramesInPreprocessorChain[i].Action                             = ActionPassOnFrame;
		FramesInPreprocessorChain[i].EmptyReleaseReferenceFrameStash    = true;
		FramesInPreprocessorChain[i].CodedBuffer                        = CodedBuffer;
		FramesInPreprocessorChain[i].ParsedFrameParameters              = ParsedFrameParameters;
		FramesInPreprocessorChain[i].DecodeFrameIndex                   = ParsedFrameParameters->DecodeFrameIndex;

		CodedBuffer->IncrementReferenceCount( IdentifierH264Intermediate );
		FramesInPreprocessorChainRing->Insert( i );
		OS_UnLockMutex( &H264Lock );
		return CodecNoError;
	    }

	report( severity_error, "Codec_MmeVideoH264_c::Input - Failed to find an entry in the \"FramesInPreprocessorChain\" list.\n" );
	OS_UnLockMutex( &H264Lock );
	return CodecError;
    }

    //
    // If this is not the first slice, we check for an update
    // to the reference frame list. Strictly the reference frame
    // list cannot change, but uif the first slice was an I slice
    // the we would not have transmitted the reference frame list
    //

    if( (ParsedFrameParameters->DecodeFrameIndex == LastSeenDecodeIndex) &&
	(FramesInPreprocessorChain[AccumulatingFrame].ParsedFrameParameters->ReferenceFrameList[0].EntryCount == 0) &&
	(ParsedFrameParameters->ReferenceFrameList[0].EntryCount != 0) )
    {
	memcpy( FramesInPreprocessorChain[AccumulatingFrame].ParsedFrameParameters->ReferenceFrameList,
		ParsedFrameParameters->ReferenceFrameList,
		H264_NUM_REF_FRAME_LISTS * sizeof(ReferenceFrameList_t) );
    }

    //
    // Is this not the first slice, if so then we pass this slice
    // to the preprocessor for appending to the current frame.
    //

    CodedData           += ParsedFrameParameters->DataOffset;
    CodedDataLength     -= ParsedFrameParameters->DataOffset;

    if( ParsedFrameParameters->DecodeFrameIndex == LastSeenDecodeIndex )
    {
	PPStatus        = H264ppAppendBuffer( PreProcessorDevice, CodedDataLength, CodedData );
	if( PPStatus != h264pp_ok )
	    report( severity_error, "Codec_MmeVideoH264_c::Input - Failed to append slice to pre-processor.\n" );

	OS_UnLockMutex( &H264Lock );
	return (PPStatus == h264pp_ok) ? CodecNoError : CodecError;
    }

    //
    // We have a first slice. Do we have an accumulated
    // buffer lying around, if so fire it off.
    //

    FireOffPreProcessing();

    //
    // We believe we have a first slice. Do we actually have one
    //

    FrameParameters     = (H264FrameParameters_t *)ParsedFrameParameters->FrameParameterStructure;

    if( !ParsedVideoParameters->FirstSlice )
    {
// 	report( severity_error, "Codec_MmeVideoH264_c::Input - Non first slice, when one is expected.\n" );
	OS_UnLockMutex( &H264Lock );
	return CodecError;
    }

    //
    // Need to prepare a new buffer, and then append the data to it.
    // First get a pre-processor buffer and attach it to the coded frame.
    // Note since it is attached to the coded frame, we can release our
    // personal hold on it.
    //

    Status      = PreProcessorBufferPool->GetBuffer( &PreProcessorBuffer );
    if( Status != BufferNoError )
    {
	report( severity_error, "Codec_MmeVideoH264_c::Input - Failed to get a pre-processor buffer (%08x).\n", Status );
	OS_UnLockMutex( &H264Lock );
	return Status;
    }

    CodedBuffer->AttachBuffer( PreProcessorBuffer );
    PreProcessorBuffer->DecrementReferenceCount();

    //
    // Now get an entry into the frames list, and fill in the relevent fields
    //

    for( i=0; i<H264_CODED_FRAME_COUNT; i++ )
	if( !FramesInPreprocessorChain[i].Used )
	    break;

    if( i == H264_CODED_FRAME_COUNT )
    {
	report( severity_error, "Codec_MmeVideoH264_c::Input - Failed to find an entry in the \"FramesInPreprocessorChain\" list - Implementation error.\n" );
	OS_UnLockMutex( &H264Lock );
	return PlayerImplementationError;
    }

//

    memset( &FramesInPreprocessorChain[i], 0x00, sizeof(FramesInPreprocessorChain_t) );
    FramesInPreprocessorChain[i].Used                                   = true;
    FramesInPreprocessorChain[i].Action                                 = ActionPassOnPreProcessedFrame;
    FramesInPreprocessorChain[i].EmptyReleaseReferenceFrameStash        = true;
    FramesInPreprocessorChain[i].CodedBuffer                            = CodedBuffer;
    FramesInPreprocessorChain[i].PreProcessorBuffer                     = PreProcessorBuffer;
    FramesInPreprocessorChain[i].ParsedFrameParameters                  = ParsedFrameParameters;
    FramesInPreprocessorChain[i].DecodeFrameIndex                       = ParsedFrameParameters->DecodeFrameIndex;

    PreProcessorBuffer->ObtainDataReference( NULL, NULL, (void **)(&FramesInPreprocessorChain[i].BufferCachedAddress), CachedAddress );
    PreProcessorBuffer->ObtainDataReference( NULL, NULL, (void **)(&FramesInPreprocessorChain[i].BufferPhysicalAddress), PhysicalAddress );

    CodedBuffer->IncrementReferenceCount( IdentifierH264Intermediate );

    //
    // Inform the preprocessor
    //

    PPStatus    = H264ppPrepareBuffer(  PreProcessorDevice,
					&FrameParameters->SliceHeader,
					i,
					FramesInPreprocessorChain[i].BufferCachedAddress,
					FramesInPreprocessorChain[i].BufferPhysicalAddress );
    if( PPStatus != h264pp_ok )
    {
	report( severity_error, "Codec_MmeVideoH264_c::Input - Failed to prepare a buffer, Junking frame.\n" );
	FramesInPreprocessorChain[i].Used               = false;
	CodedBuffer->DecrementReferenceCount( IdentifierH264Intermediate );
	OS_UnLockMutex( &H264Lock );
	return CodecError;
    }

    //
    // Now append the data then shrink the buffer to release the dataspace
    //

    PPStatus    = H264ppAppendBuffer( PreProcessorDevice, CodedDataLength, CodedData );
    if( PPStatus != h264pp_ok )
    {
	report( severity_error, "Codec_MmeVideoH264_c::Input - Failed to append first slice to pre-processor, Junking frame.\n" );
	FramesInPreprocessorChain[i].Used               = false;
	CodedBuffer->DecrementReferenceCount( IdentifierH264Intermediate );
	OS_UnLockMutex( &H264Lock );
	return CodecError;
    }

//

    LastSeenDecodeIndex         = ParsedFrameParameters->DecodeFrameIndex;
    AccumulatingFrame           = i;
    OS_UnLockMutex( &H264Lock );

//

    CodedBuffer->SetUsedDataSize( 0 );

    Status	= CodedBuffer->ShrinkBuffer( 0 );
    if( Status != BufferNoError )
	report( severity_info, "Codec_MmeVideoH264_c::Input - Failed to shrink buffer.\n" );

//

    return CodecNoError;
}


// /////////////////////////////////////////////////////////////////////////
//
//      Function to flush any accumulated slices through the pre-processor
//      used on a first slice, but also when we think we have a full frame
//      ie when releasing a reference frame etc.
//


CodecStatus_t   Codec_MmeVideoH264_c::FireOffPreProcessing( bool	Speculative )
{
h264pp_status_t           PPStatus;

//

    if( AccumulatingFrame != INVALID_INDEX )
    {
	FramesInPreprocessorChain[AccumulatingFrame].SpeculativePreProcess	= Speculative;

	PPStatus        = H264ppProcessBuffer( PreProcessorDevice );
	if( PPStatus != h264pp_ok )
	{
	    report( severity_error, "Codec_MmeVideoH264_c::FireOffPreProcessing - Failed to fire off pre-processor, Junking frame.\n" );
	    FramesInPreprocessorChain[AccumulatingFrame].CodedBuffer->DecrementReferenceCount( IdentifierH264Intermediate );
	    FramesInPreprocessorChain[AccumulatingFrame].Used           = false;
	}
	else
	    FramesInPreprocessorChainRing->Insert( AccumulatingFrame );

	AccumulatingFrame       = INVALID_INDEX;
	LastSeenDecodeIndex     = INVALID_INDEX;
    }

    return CodecNoError;
}


// /////////////////////////////////////////////////////////////////////////
//
//      Function to deal with the returned capabilities
//      structure for an H264 mme transformer.
//

CodecStatus_t   Codec_MmeVideoH264_c::HandleCapabilities( void )
{
    report( severity_info, "MME Transformer '%s' capabilities are :-\n", H264_MME_TRANSFORMER_NAME );
    report( severity_info, "    SD_MaxMBStructureSize             = %d bytes\n", H264TransformCapability.SD_MaxMBStructureSize );
    report( severity_info, "    HD_MaxMBStructureSize             = %d bytes\n", H264TransformCapability.HD_MaxMBStructureSize );
    report( severity_info, "    MaximumFieldDecodingLatency90KHz  = %d\n", H264TransformCapability.MaximumFieldDecodingLatency90KHz );

    //
    // Unless things have changed, the firmware always
    // reported zero for the sizes, so we use our own
    // knowm values.
    //

    SD_MaxMBStructureSize		= 160;
    HD_MaxMBStructureSize		= 64;
    report( severity_info, "    Using default MB structure sizes   = %d, %d bytes\n", SD_MaxMBStructureSize, HD_MaxMBStructureSize );

    return CodecNoError;
}


// /////////////////////////////////////////////////////////////////////////
//
//      Function to deal with the returned capabilities
//      structure for an H264 mme transformer.
//

CodecStatus_t   Codec_MmeVideoH264_c::FillOutTransformerInitializationParameters( void )
{

    //
    // Fillout the command parameters
    //

    H264InitializationParameters.CircularBinBufferBeginAddr_p   = (U32 *)0x00000000;
    H264InitializationParameters.CircularBinBufferEndAddr_p     = (U32 *)0xffffffff;

    //
    // Fillout the actual command
    //

    MMEInitializationParameters.TransformerInitParamsSize       = sizeof(H264_InitTransformerParam_fmw_t);
    MMEInitializationParameters.TransformerInitParams_p         = (MME_GenericParams_t)(&H264InitializationParameters);

//

    return CodecNoError;
}


// /////////////////////////////////////////////////////////////////////////
//
//      Function to fill out the stream parameters
//      structure for an H264 mme transformer.
//

CodecStatus_t   Codec_MmeVideoH264_c::FillOutSetStreamParametersCommand( void )
{
H264StreamParameters_t                  *Parsed         = (H264StreamParameters_t *)ParsedFrameParameters->StreamParameterStructure;
H264CodecStreamParameterContext_t       *Context        = (H264CodecStreamParameterContext_t *)StreamParameterContext;
H264PictureParameterSetHeader_t         *PPS;
H264SequenceParameterSetHeader_t        *SPS;

    //
    // Fillout the command parameters
    //

    SPS         = Parsed->SequenceParameterSet;
    PPS         = Parsed->PictureParameterSet;

//

    memset( &Context->StreamParameters, 0x00, sizeof(H264_SetGlobalParam_t) );

    Context->StreamParameters.H264SetGlobalParamSPS.DecoderProfileConformance          = H264_HIGH_PROFILE;
//    Context->StreamParameters.H264SetGlobalParamSPS.DecoderProfileConformance          = (H264_DecoderProfileComformance_t)SPS->profile_idc;

    Context->StreamParameters.H264SetGlobalParamSPS.level_idc                          = SPS->level_idc;
    Context->StreamParameters.H264SetGlobalParamSPS.log2_max_frame_num_minus4          = SPS->log2_max_frame_num_minus4;
    Context->StreamParameters.H264SetGlobalParamSPS.pic_order_cnt_type                 = SPS->pic_order_cnt_type;
    Context->StreamParameters.H264SetGlobalParamSPS.log2_max_pic_order_cnt_lsb_minus4  = SPS->log2_max_pic_order_cnt_lsb_minus4;
    Context->StreamParameters.H264SetGlobalParamSPS.delta_pic_order_always_zero_flag   = SPS->delta_pic_order_always_zero_flag;
    Context->StreamParameters.H264SetGlobalParamSPS.pic_width_in_mbs_minus1            = SPS->pic_width_in_mbs_minus1;
    Context->StreamParameters.H264SetGlobalParamSPS.pic_height_in_map_units_minus1     = SPS->pic_height_in_map_units_minus1;
    Context->StreamParameters.H264SetGlobalParamSPS.frame_mbs_only_flag                = SPS->frame_mbs_only_flag;
    Context->StreamParameters.H264SetGlobalParamSPS.mb_adaptive_frame_field_flag       = SPS->mb_adaptive_frame_field_flag;
    Context->StreamParameters.H264SetGlobalParamSPS.direct_8x8_inference_flag          = SPS->direct_8x8_inference_flag;
    Context->StreamParameters.H264SetGlobalParamSPS.chroma_format_idc                  = SPS->chroma_format_idc;

    Context->StreamParameters.H264SetGlobalParamPPS.entropy_coding_mode_flag           = PPS->entropy_coding_mode_flag;
    Context->StreamParameters.H264SetGlobalParamPPS.pic_order_present_flag             = PPS->pic_order_present_flag;
    Context->StreamParameters.H264SetGlobalParamPPS.num_ref_idx_l0_active_minus1       = PPS->num_ref_idx_l0_active_minus1;
    Context->StreamParameters.H264SetGlobalParamPPS.num_ref_idx_l1_active_minus1       = PPS->num_ref_idx_l1_active_minus1;
    Context->StreamParameters.H264SetGlobalParamPPS.weighted_pred_flag                 = PPS->weighted_pred_flag;
    Context->StreamParameters.H264SetGlobalParamPPS.weighted_bipred_idc                = PPS->weighted_bipred_idc;
    Context->StreamParameters.H264SetGlobalParamPPS.pic_init_qp_minus26                = PPS->pic_init_qp_minus26;
    Context->StreamParameters.H264SetGlobalParamPPS.chroma_qp_index_offset             = PPS->chroma_qp_index_offset;
    Context->StreamParameters.H264SetGlobalParamPPS.deblocking_filter_control_present_flag = PPS->deblocking_filter_control_present_flag;
    Context->StreamParameters.H264SetGlobalParamPPS.constrained_intra_pred_flag        = PPS->constrained_intra_pred_flag;
    Context->StreamParameters.H264SetGlobalParamPPS.transform_8x8_mode_flag            = PPS->transform_8x8_mode_flag;
    Context->StreamParameters.H264SetGlobalParamPPS.second_chroma_qp_index_offset      = PPS->second_chroma_qp_index_offset;

    Context->StreamParameters.H264SetGlobalParamPPS.ScalingList.ScalingListUpdated     = 1;

    if( PPS->pic_scaling_matrix_present_flag )
    {
	memcpy( Context->StreamParameters.H264SetGlobalParamPPS.ScalingList.FirstSixScalingList, PPS->ScalingList4x4, sizeof(PPS->ScalingList4x4) );
	memcpy( Context->StreamParameters.H264SetGlobalParamPPS.ScalingList.NextTwoScalingList, PPS->ScalingList8x8, sizeof(PPS->ScalingList8x8) );
    }
    else
    {
	memcpy( Context->StreamParameters.H264SetGlobalParamPPS.ScalingList.FirstSixScalingList, SPS->ScalingList4x4, sizeof(PPS->ScalingList4x4) );
	memcpy( Context->StreamParameters.H264SetGlobalParamPPS.ScalingList.NextTwoScalingList, SPS->ScalingList8x8, sizeof(PPS->ScalingList8x8) );
    }

    //
    // Fillout the actual command
    //

    memset( &Context->BaseContext.MMECommand, 0x00, sizeof(MME_Command_t) );

    Context->BaseContext.MMECommand.CmdStatus.AdditionalInfoSize        = 0;
    Context->BaseContext.MMECommand.CmdStatus.AdditionalInfo_p          = NULL;
    Context->BaseContext.MMECommand.ParamSize                           = sizeof(H264_SetGlobalParam_t);
    Context->BaseContext.MMECommand.Param_p                             = (MME_GenericParams_t)(&Context->StreamParameters);

//

    return CodecNoError;
}


// /////////////////////////////////////////////////////////////////////////
//
//      Function to fill out the decode parameters
//      structure for an H264 mme transformer.
//

CodecStatus_t   Codec_MmeVideoH264_c::FillOutDecodeCommand(       void )
{
CodecStatus_t                     Status;
H264CodecDecodeContext_t         *Context                       = (H264CodecDecodeContext_t *)DecodeContext;
H264_TransformParam_fmw_t        *Param;
H264SequenceParameterSetHeader_t *SPS;
unsigned int			  MacroBlockStructureSize;
Buffer_t                          MacroBlockStructureBuffer;
Buffer_t                          PreProcessorBuffer;
unsigned int                      InputBuffer;
unsigned int                      PreProcessorBufferSize;
bool				  DowngradeDecode;
unsigned int			  MaxAllocatableDecodeBuffers;

    //
    // For H264 we do not do slice decodes.
    //

    KnownLastSliceInFieldFrame                  = true;

    //
    // If this is a reference frame, and no macroblock structure has been attached
    // to this decode buffer, obtain a reference frame slot, and obtain a macroblock
    // structure buffer and attach it.
    //

    if( ParsedFrameParameters->ReferenceFrame &&
	(BufferState[CurrentDecodeBufferIndex].BufferMacroblockStructurePointer == NULL) )
    {
	//
	// Obtain a reference frame slot
	//

	Status	= ReferenceFrameSlotAllocate( CurrentDecodeBufferIndex );
	if( Status != CodecNoError )
	{
#ifdef __TDT__
	    report( severity_error, "\n\nCodec_MmeVideoH264_c::FillOutDecodeCommand - Failed to obtain a reference frame slot.\n\n\n" );
	    /* This is necessary to avoid deadlocks caused by lack
	       of further reference frame slots. (There is a firmware
	       limitation of 16 reference frames.) It is unclear
               what exactly causes the firmware to stop releasing
	       the buffers with reference frames. Sometimes it happens
	       during in live stream (e.g. in some showcase loops) and
	       sometimes it might be caused by synchronization problems.
	       I tried lots of additional actions (like discarding
	       manifestor decode buffers) to speed up the resynchronization
	       but most of them lead to another deadlock. */
	    H264ReleaseReferenceFrame( CODEC_RELEASE_ALL);
#else
	    report( severity_error, "Codec_MmeVideoH264_c::FillOutDecodeCommand - Failed to obtain a reference frame slot.\n" );
#endif
	    return Status;
	}

	//
	// Obtain a macroblock structure buffer
	//    First calculate the size,
	//    Second verify that there are enough decode buffers for the reference frame count
	//    Third verify that the macroblock structure memory is large enough
	//    Fourth allocate the actual buffer.
	//

	SPS         = ((H264FrameParameters_t *)ParsedFrameParameters->FrameParameterStructure)->SliceHeader.SequenceParameterSet;

	MacroBlockStructureSize		 = 4 * ((ParsedVideoParameters->Content.Width + 31) / 32) * ((ParsedVideoParameters->Content.Height + 31) / 32);
	MacroBlockStructureSize		*= (SPS->level_idc <= 30) ? SD_MaxMBStructureSize : HD_MaxMBStructureSize;

	Manifestor->GetDecodeBufferCount( &MaxAllocatableDecodeBuffers );
	if( SPS->num_ref_frames > (MaxAllocatableDecodeBuffers - 3) )
	{
	    report( severity_error, "Codec_MmeVideoH264_c::FillOutDecodeCommand - Insufficient memory allocated to decode buffers to cope with reference frame count and size (%d %d).\n",
		    SPS->num_ref_frames, MaxAllocatableDecodeBuffers );
	    Player->MarkStreamUnPlayable( Stream );
	    return CodecError;
	}

	if( SPS->num_ref_frames > ((H264_MACROBLOCK_STRUCTURE_MEMORY/MacroBlockStructureSize) - 3) )
	{
	    report( severity_error, "Codec_MmeVideoH264_c::FillOutDecodeCommand - Insufficient memory allocated to macro block structures to cope with reference frame count and size (%08x - %d %08x).\n",
		    H264_MACROBLOCK_STRUCTURE_MEMORY, SPS->num_ref_frames, MacroBlockStructureSize );
	    Player->MarkStreamUnPlayable( Stream );
	    return CodecError;
	}

	Status  = MacroBlockStructurePool->GetBuffer( &MacroBlockStructureBuffer, UNSPECIFIED_OWNER, MacroBlockStructureSize );
	if( Status != BufferNoError )
	{
	    report( severity_error, "Codec_MmeVideoH264_c::FillOutDecodeCommand - Failed to get macroblock structure buffer.\n" );
	    return Status;
	}

	MacroBlockStructureBuffer->ObtainDataReference( NULL, NULL, (void **)&BufferState[CurrentDecodeBufferIndex].BufferMacroblockStructurePointer, PhysicalAddress );

	// Attach it to the decode buffer, since the decode buffer holds it, we can release our hold.

	BufferState[CurrentDecodeBufferIndex].Buffer->AttachBuffer( MacroBlockStructureBuffer );
	MacroBlockStructureBuffer->DecrementReferenceCount();
    }

    //
    // Detach the pre-processor buffer and re-attach it to the decode context
    // this will make its release automatic on the freeing of the decode context.
    //

    Status      = CodedFrameBuffer->ObtainAttachedBufferReference( PreProcessorBufferType, &PreProcessorBuffer );
    if( Status != BufferNoError )
    {
	report( severity_error, "Codec_MmeVideoH264_c::FillOutDecodeCommand - Failed to get the pre-processed buffer.\n" );
	return Status;
    }

    DecodeContextBuffer->AttachBuffer( PreProcessorBuffer );    // Must be ordered, otherwise the pre-processor
    CodedFrameBuffer->DetachBuffer( PreProcessorBuffer );       // buffer will get released in the middle

    //
    // Fill out the sub-components of the command data
    //

    memset( &Context->DecodeParameters, 0x00, sizeof(H264_TransformParam_fmw_t) );
    memset( &Context->DecodeStatus, 0xa5, sizeof(H264_CommandStatus_fmw_t) );

    FillOutDecodeCommandHostData();
    FillOutDecodeCommandRefPicList();

    //
    // Fill out the remaining fields in the transform parameters structure.
    //

    Param                                       = &Context->DecodeParameters;
    PreProcessorBuffer->ObtainDataReference( NULL, &PreProcessorBufferSize, (void **)&InputBuffer, PhysicalAddress );

    Param->PictureStartAddrBinBuffer            = (unsigned int *)InputBuffer;                         		// Start address in the bin buffer
    Param->PictureStopAddrBinBuffer             = (unsigned int *)(InputBuffer + PreProcessorBufferSize);   	// Stop address in the bin buffer
    Param->MaxSESBSize                          = H264_PP_SESB_SIZE;

    Param->MainAuxEnable                        = MAINOUT_EN;                                           // Enable only the main decode
    Param->HorizontalDecimationFactor           = HDEC_1;                                               // Do not decimate
    Param->VerticalDecimationFactor             = VDEC_1;

    DowngradeDecode				= ParsedFrameParameters->ApplySubstandardDecode &&
		SLICE_TYPE_IS(((H264FrameParameters_t *)ParsedFrameParameters->FrameParameterStructure)->SliceHeader.slice_type, H264_SLICE_TYPE_B );

    Param->DecodingMode                         = DowngradeDecode ? H264_DOWNGRADED_DECODE_LEVEL1 : H264_NORMAL_DECODE;
    Param->AdditionalFlags                      = H264_ADDITIONAL_FLAG_NONE;                            // No future use yet identified

    Param->DecodedBufferAddress.Luma_p          = (H264_LumaAddress_t)BufferState[CurrentDecodeBufferIndex].BufferLumaPointer;
    Param->DecodedBufferAddress.Chroma_p        = (H264_ChromaAddress_t)BufferState[CurrentDecodeBufferIndex].BufferChromaPointer;
    Param->DecodedBufferAddress.MBStruct_p      = (H264_MBStructureAddress_t)BufferState[CurrentDecodeBufferIndex].BufferMacroblockStructurePointer;

    //
    // Fillout the actual command
    //

    memset( &Context->BaseContext.MMECommand, 0x00, sizeof(MME_Command_t) );

    Context->BaseContext.MMECommand.CmdStatus.AdditionalInfoSize        = sizeof(H264_CommandStatus_fmw_t);
    Context->BaseContext.MMECommand.CmdStatus.AdditionalInfo_p          = (MME_GenericParams_t)(&Context->DecodeStatus);
    Context->BaseContext.MMECommand.ParamSize                           = sizeof(H264_TransformParam_fmw_t);
    Context->BaseContext.MMECommand.Param_p                             = (MME_GenericParams_t)(&Context->DecodeParameters);

    return CodecNoError;
}


// /////////////////////////////////////////////////////////////////////////
//
//      Function to fill out the host data segment of the h264 decode
//      parameters structure for the H264 mme transformer.
//

CodecStatus_t   Codec_MmeVideoH264_c::FillOutDecodeCommandHostData( void )
{
H264CodecDecodeContext_t                 *Context               = (H264CodecDecodeContext_t *)DecodeContext;
H264FrameParameters_t                    *Parsed                = (H264FrameParameters_t *)ParsedFrameParameters->FrameParameterStructure;
H264SequenceParameterSetHeader_t         *SPS;
H264SliceHeader_s                        *SliceHeader;
H264_HostData_t                          *HostData;

//

    SliceHeader = &Parsed->SliceHeader;
    SPS         = SliceHeader->SequenceParameterSet;
    HostData    = &Context->DecodeParameters.HostData;

//

    HostData->PictureNumber             = 0;
    HostData->PicOrderCntTop            = SliceHeader->PicOrderCntTop;
    HostData->PicOrderCntBot            = SliceHeader->PicOrderCntBot;

    if( SPS->frame_mbs_only_flag || !SliceHeader->field_pic_flag )
    {
	HostData->PicOrderCnt           = min( SliceHeader->PicOrderCntTop, SliceHeader->PicOrderCntBot );
	HostData->DescriptorType        = SPS->mb_adaptive_frame_field_flag ? H264_PIC_DESCRIPTOR_AFRAME : H264_PIC_DESCRIPTOR_FRAME;
    }
    else
    {
	HostData->PicOrderCnt           = SliceHeader->bottom_field_flag ? SliceHeader->PicOrderCntBot : SliceHeader->PicOrderCntTop;
	HostData->DescriptorType        = SliceHeader->bottom_field_flag ? H264_PIC_DESCRIPTOR_FIELD_BOTTOM : H264_PIC_DESCRIPTOR_FIELD_TOP;
    }

    HostData->ReferenceType             = (SliceHeader->nal_ref_idc != 0) ?                             // Its a reference frame
						(SliceHeader->dec_ref_pic_marking.long_term_reference_flag ?
							H264_LONG_TERM_REFERENCE :
							H264_SHORT_TERM_REFERENCE) :
							H264_UNUSED_FOR_REFERENCE;
    HostData->IdrFlag                   = SliceHeader->nal_unit_type == NALU_TYPE_IDR;
    HostData->NonExistingFlag           = 0;

//

    BufferState[CurrentDecodeBufferIndex].DecodedAsFrames       = (SliceHeader->field_pic_flag == 0);
    BufferState[CurrentDecodeBufferIndex].DecodedWithMbaff      = (SPS->mb_adaptive_frame_field_flag != 0);

    //
    // Save the host data only if this is a reference frame
    //

    if( ParsedFrameParameters->ReferenceFrame )
	memcpy( &RecordedHostData[CurrentDecodeBufferIndex], HostData, sizeof(H264_HostData_t) );

//

    return CodecNoError;
}


// /////////////////////////////////////////////////////////////////////////
//
//      Function to fill out the reference picture list segment of the
//      h264 decode parameters structure for the H264 mme transformer.
//

CodecStatus_t   Codec_MmeVideoH264_c::FillOutDecodeCommandRefPicList( void )
{
unsigned int                              i;
H264CodecDecodeContext_t                 *Context               = (H264CodecDecodeContext_t *)DecodeContext;
H264FrameParameters_t                    *Parsed                = (H264FrameParameters_t *)ParsedFrameParameters->FrameParameterStructure;
H264SequenceParameterSetHeader_t         *SPS;
H264_RefPictListAddress_t                *RefPicLists;
unsigned int                              ReferenceId;
unsigned int				  Slot;
unsigned int                              DescriptorIndex;

//

    SPS         = Parsed->SliceHeader.SequenceParameterSet;
    RefPicLists = &Context->DecodeParameters.InitialRefPictList;

//

    //
    // Setup the frame address array
    //

    for( i=0; i<DecodeBufferCount; i++ )
	if( (BufferState[i].ReferenceFrameCount != 0) && (BufferState[i].ReferenceFrameSlot != INVALID_INDEX) )
	{
	    Slot	= BufferState[i].ReferenceFrameSlot;

	    RefPicLists->ReferenceFrameAddress[Slot].DecodedBufferAddress.Luma_p       = (H264_LumaAddress_t)BufferState[i].BufferLumaPointer;
	    RefPicLists->ReferenceFrameAddress[Slot].DecodedBufferAddress.Chroma_p     = (H264_ChromaAddress_t)BufferState[i].BufferChromaPointer;
	    RefPicLists->ReferenceFrameAddress[Slot].DecodedBufferAddress.MBStruct_p   = (H264_MBStructureAddress_t)BufferState[i].BufferMacroblockStructurePointer;
	}

    //
    // Create the reference picture lists, NOTE this is a fudge, the
    // delta-phi requires reference picture lists for B and P pictures
    // to be supplied for all pictures (thats I B or P), so we have to
    // make up reference picture lists for a frame that this isnt.
    //

    NumberOfUsedDescriptors     = 0;
    memset( DescriptorIndices, 0xff, sizeof(DescriptorIndices) );

    RefPicLists->ReferenceDescriptorsBitsField  = 0;                            // Default to all being frame descriptors

    //
    // For each entry in the lists we check if the descriptor is already available
    // and point to it. If it is not available we create a new descriptor.
    //

    RefPicLists->InitialPList0.RefPiclistSize   = DecodeContext->ReferenceFrameList[P_REF_PIC_LIST].EntryCount;
    for( i=0; i<RefPicLists->InitialPList0.RefPiclistSize; i++ )
    {
	unsigned int    UsageCode                       = DecodeContext->ReferenceFrameList[P_REF_PIC_LIST].H264ReferenceDetails[i].UsageCode;
	unsigned int    BufferIndex                     = DecodeContext->ReferenceFrameList[P_REF_PIC_LIST].EntryIndicies[i];

	Slot						= BufferState[BufferIndex].ReferenceFrameSlot;
	ReferenceId                                     = (3 * Slot) + UsageCode;
	DescriptorIndex                                 = DescriptorIndices[ReferenceId];

	if( DescriptorIndex == 0xff )
	    DescriptorIndex                             = FillOutNewDescriptor( ReferenceId, BufferIndex, &DecodeContext->ReferenceFrameList[P_REF_PIC_LIST].H264ReferenceDetails[i] );

	RefPicLists->InitialPList0.RefPicList[i]        = DescriptorIndex;
    }

//

    RefPicLists->InitialBList0.RefPiclistSize   = DecodeContext->ReferenceFrameList[B_REF_PIC_LIST_0].EntryCount;
    for( i=0; i<RefPicLists->InitialBList0.RefPiclistSize; i++ )
    {
	unsigned int    UsageCode                       = DecodeContext->ReferenceFrameList[B_REF_PIC_LIST_0].H264ReferenceDetails[i].UsageCode;
	unsigned int    BufferIndex                     = DecodeContext->ReferenceFrameList[B_REF_PIC_LIST_0].EntryIndicies[i];

	Slot						= BufferState[BufferIndex].ReferenceFrameSlot;
	ReferenceId                                     = (3 * Slot) + UsageCode;
	DescriptorIndex                                 = DescriptorIndices[ReferenceId];

	if( DescriptorIndex == 0xff )
	    DescriptorIndex                             = FillOutNewDescriptor( ReferenceId, BufferIndex, &DecodeContext->ReferenceFrameList[B_REF_PIC_LIST_0].H264ReferenceDetails[i] );

	RefPicLists->InitialBList0.RefPicList[i]        = DescriptorIndex;
    }

//

    RefPicLists->InitialBList1.RefPiclistSize   = DecodeContext->ReferenceFrameList[B_REF_PIC_LIST_1].EntryCount;
    for( i=0; i<RefPicLists->InitialBList1.RefPiclistSize; i++ )
    {
	unsigned int    UsageCode                       = DecodeContext->ReferenceFrameList[B_REF_PIC_LIST_1].H264ReferenceDetails[i].UsageCode;
	unsigned int    BufferIndex                     = DecodeContext->ReferenceFrameList[B_REF_PIC_LIST_1].EntryIndicies[i];

	Slot						= BufferState[BufferIndex].ReferenceFrameSlot;
	ReferenceId                                     = (3 * Slot) + UsageCode;
	DescriptorIndex                                 = DescriptorIndices[ReferenceId];

	if( DescriptorIndex == 0xff )
	    DescriptorIndex                             = FillOutNewDescriptor( ReferenceId, BufferIndex, &DecodeContext->ReferenceFrameList[B_REF_PIC_LIST_1].H264ReferenceDetails[i] );

	RefPicLists->InitialBList1.RefPicList[i]        = DescriptorIndex;
    }

//

    return CodecNoError;
}


// /////////////////////////////////////////////////////////////////////////
//
//      Function to fill out a single descriptor for a h264 reference frame
//

unsigned int   Codec_MmeVideoH264_c::FillOutNewDescriptor(
							unsigned int             ReferenceId,
							unsigned int             BufferIndex,
							H264ReferenceDetails_t  *Details )
{
H264CodecDecodeContext_t                 *Context               = (H264CodecDecodeContext_t *)DecodeContext;
H264_RefPictListAddress_t                *RefPicLists;
unsigned int                              DescriptorIndex;
unsigned int				  SlotIndex;
H264_PictureDescriptor_t                 *Descriptor;

//

    RefPicLists = &Context->DecodeParameters.InitialRefPictList;

    DescriptorIndex                     = NumberOfUsedDescriptors++;
    SlotIndex				= BufferState[BufferIndex].ReferenceFrameSlot;

    DescriptorIndices[ReferenceId]      = DescriptorIndex;
    Descriptor                          = &RefPicLists->PictureDescriptor[DescriptorIndex];
    Descriptor->FrameIndex              = SlotIndex;

    //
    // Setup the host data
    //

    memcpy( &Descriptor->HostData, &RecordedHostData[BufferIndex], sizeof(H264_HostData_t) );

    Descriptor->HostData.PictureNumber  = Details->PictureNumber;
    Descriptor->HostData.PicOrderCnt    = Details->PicOrderCnt;
    Descriptor->HostData.PicOrderCntTop = Details->PicOrderCntTop;
    Descriptor->HostData.PicOrderCntBot = Details->PicOrderCntBot;
    Descriptor->HostData.ReferenceType  = Details->LongTermReference ? H264_LONG_TERM_REFERENCE : H264_SHORT_TERM_REFERENCE;

    //
    // The nature of the frame being referenced is in its original descriptor type
    //

    if( (Descriptor->HostData.DescriptorType != H264_PIC_DESCRIPTOR_FRAME) &&
	(Descriptor->HostData.DescriptorType != H264_PIC_DESCRIPTOR_AFRAME) )
	RefPicLists->ReferenceDescriptorsBitsField      |= 1<<SlotIndex;

    //
    // We now modify the descriptor type to reflect the way in which we reference the frame
    //

    if( Details->UsageCode == REF_PIC_USE_FRAME )
    {
	if( (Descriptor->HostData.DescriptorType != H264_PIC_DESCRIPTOR_AFRAME) &&
	    (Descriptor->HostData.DescriptorType != H264_PIC_DESCRIPTOR_FRAME) )
	{
//          Descriptor->HostData.DescriptorType = BufferState[BufferIndex].DecodedWithMbaff ? H264_PIC_DESCRIPTOR_AFRAME : H264_PIC_DESCRIPTOR_FRAME;
	    Descriptor->HostData.DescriptorType = H264_PIC_DESCRIPTOR_FRAME;
	}
    }
    else if( Details->UsageCode == REF_PIC_USE_FIELD_TOP )
	Descriptor->HostData.DescriptorType     = H264_PIC_DESCRIPTOR_FIELD_TOP;
    else if( Details->UsageCode == REF_PIC_USE_FIELD_BOT )
	Descriptor->HostData.DescriptorType     = H264_PIC_DESCRIPTOR_FIELD_BOTTOM;

    //
    // And return the index
    //

    return DescriptorIndex;
}

////////////////////////////////////////////////////////////////////////////
///
/// Unconditionally return success.
///
/// Success and failure codes are located entirely in the generic MME structures
/// allowing the super-class to determine whether the decode was successful. This
/// means that we have no work to do here.
///
/// \return CodecNoError
///
CodecStatus_t   Codec_MmeVideoH264_c::ValidateDecodeContext( CodecBaseDecodeContext_t *Context )
{
H264CodecDecodeContext_t	*H264Context	= (H264CodecDecodeContext_t *)Context;

#ifdef MULTICOM
//report( severity_info, "Decode took %6dus\n", H264Context->DecodeStatus.DecodeTimeInMicros );

    if( H264Context->DecodeStatus.ErrorCode != H264_DECODER_NO_ERROR )
    {
	report( severity_info, "H264 decode error %08x\n", H264Context->DecodeStatus.ErrorCode );

#if 0
	for(unsigned int i=0; i<H264_STATUS_PARTITION; i++ )
	for( i=0; i<H264_STATUS_PARTITION; i++ )
	    report( severity_info, "\t\t %02x %02x %02x %02x %02x %02x\n",
		H264Context->DecodeStatus.Status[i][0], H264Context->DecodeStatus.Status[i][1],
		H264Context->DecodeStatus.Status[i][2], H264Context->DecodeStatus.Status[i][3],
		H264Context->DecodeStatus.Status[i][4], H264Context->DecodeStatus.Status[i][5] );
	DumpMMECommand( &Context->MMECommand );
#endif
    }
#endif

    return CodecNoError;
}

// /////////////////////////////////////////////////////////////////////////
//
//      Function to dump out the set stream
//      parameters from an mme command.
//

CodecStatus_t   Codec_MmeVideoH264_c::DumpSetStreamParameters(         void    *Parameters )
{
unsigned int             i;
H264_SetGlobalParam_t   *StreamParams;

//

    StreamParams    = (H264_SetGlobalParam_t *)Parameters;

    report( severity_info,  "AZA - STREAM PARAMS %08x\n", StreamParams );
    report( severity_info,  "AZA -    SPS\n" );
    report( severity_info,  "AZA -       DecoderProfileConformance            = %d\n", StreamParams->H264SetGlobalParamSPS.DecoderProfileConformance );
    report( severity_info,  "AZA -       level_idc                            = %d\n", StreamParams->H264SetGlobalParamSPS.level_idc );
    report( severity_info,  "AZA -       log2_max_frame_num_minus4            = %d\n", StreamParams->H264SetGlobalParamSPS.log2_max_frame_num_minus4 );
    report( severity_info,  "AZA -       pic_order_cnt_type                   = %d\n", StreamParams->H264SetGlobalParamSPS.pic_order_cnt_type );
    report( severity_info,  "AZA -       log2_max_pic_order_cnt_lsb_minus4    = %d\n", StreamParams->H264SetGlobalParamSPS.log2_max_pic_order_cnt_lsb_minus4 );
    report( severity_info,  "AZA -       delta_pic_order_always_zero_flag     = %d\n", StreamParams->H264SetGlobalParamSPS.delta_pic_order_always_zero_flag );
    report( severity_info,  "AZA -       pic_width_in_mbs_minus1              = %d\n", StreamParams->H264SetGlobalParamSPS.pic_width_in_mbs_minus1 );
    report( severity_info,  "AZA -       pic_height_in_map_units_minus1       = %d\n", StreamParams->H264SetGlobalParamSPS.pic_height_in_map_units_minus1 );
    report( severity_info,  "AZA -       frame_mbs_only_flag                  = %d\n", StreamParams->H264SetGlobalParamSPS.frame_mbs_only_flag );
    report( severity_info,  "AZA -       mb_adaptive_frame_field_flag         = %d\n", StreamParams->H264SetGlobalParamSPS.mb_adaptive_frame_field_flag );
    report( severity_info,  "AZA -       direct_8x8_inference_flag            = %d\n", StreamParams->H264SetGlobalParamSPS.direct_8x8_inference_flag );
    report( severity_info,  "AZA -       chroma_format_idc                    = %d\n", StreamParams->H264SetGlobalParamSPS.chroma_format_idc );
    report( severity_info,  "AZA -    PPS\n" );
    report( severity_info,  "AZA -       entropy_coding_mode_flag             = %d\n", StreamParams->H264SetGlobalParamPPS.entropy_coding_mode_flag );
    report( severity_info,  "AZA -       pic_order_present_flag               = %d\n", StreamParams->H264SetGlobalParamPPS.pic_order_present_flag );
    report( severity_info,  "AZA -       num_ref_idx_l0_active_minus1         = %d\n", StreamParams->H264SetGlobalParamPPS.num_ref_idx_l0_active_minus1 );
    report( severity_info,  "AZA -       num_ref_idx_l1_active_minus1         = %d\n", StreamParams->H264SetGlobalParamPPS.num_ref_idx_l1_active_minus1 );
    report( severity_info,  "AZA -       weighted_pred_flag                   = %d\n", StreamParams->H264SetGlobalParamPPS.weighted_pred_flag );
    report( severity_info,  "AZA -       weighted_bipred_idc                  = %d\n", StreamParams->H264SetGlobalParamPPS.weighted_bipred_idc );
    report( severity_info,  "AZA -       pic_init_qp_minus26                  = %d\n", StreamParams->H264SetGlobalParamPPS.pic_init_qp_minus26 );
    report( severity_info,  "AZA -       chroma_qp_index_offset               = %d\n", StreamParams->H264SetGlobalParamPPS.chroma_qp_index_offset );
    report( severity_info,  "AZA -       deblocking_filter_control_present_flag = %d\n", StreamParams->H264SetGlobalParamPPS.deblocking_filter_control_present_flag );
    report( severity_info,  "AZA -       constrained_intra_pred_flag          = %d\n", StreamParams->H264SetGlobalParamPPS.constrained_intra_pred_flag );
    report( severity_info,  "AZA -       transform_8x8_mode_flag              = %d\n", StreamParams->H264SetGlobalParamPPS.transform_8x8_mode_flag );
    report( severity_info,  "AZA -       second_chroma_qp_index_offset        = %d\n", StreamParams->H264SetGlobalParamPPS.second_chroma_qp_index_offset );
    report( severity_info,  "AZA -       ScalingListUpdated                   = %d\n", StreamParams->H264SetGlobalParamPPS.ScalingList.ScalingListUpdated );

    for( i=0; i<6; i++ )
	report( severity_info,  "AZA -       ScalingList4x4[%d]                   = %02x %02x %02x %02x %02x %02x %02x %02x\n", i,
		   StreamParams->H264SetGlobalParamPPS.ScalingList.FirstSixScalingList[i][0], StreamParams->H264SetGlobalParamPPS.ScalingList.FirstSixScalingList[i][1],
		   StreamParams->H264SetGlobalParamPPS.ScalingList.FirstSixScalingList[i][2], StreamParams->H264SetGlobalParamPPS.ScalingList.FirstSixScalingList[i][3],
		   StreamParams->H264SetGlobalParamPPS.ScalingList.FirstSixScalingList[i][4], StreamParams->H264SetGlobalParamPPS.ScalingList.FirstSixScalingList[i][5],
		   StreamParams->H264SetGlobalParamPPS.ScalingList.FirstSixScalingList[i][6], StreamParams->H264SetGlobalParamPPS.ScalingList.FirstSixScalingList[i][7] );

    for( i=0; i<2; i++ )
	report( severity_info,  "AZA -       ScalingList8x8[%d]                   = %02x %02x %02x %02x %02x %02x %02x %02x\n", i,
		   StreamParams->H264SetGlobalParamPPS.ScalingList.NextTwoScalingList[i][0], StreamParams->H264SetGlobalParamPPS.ScalingList.NextTwoScalingList[i][1],
		   StreamParams->H264SetGlobalParamPPS.ScalingList.NextTwoScalingList[i][2], StreamParams->H264SetGlobalParamPPS.ScalingList.NextTwoScalingList[i][3],
		   StreamParams->H264SetGlobalParamPPS.ScalingList.NextTwoScalingList[i][4], StreamParams->H264SetGlobalParamPPS.ScalingList.NextTwoScalingList[i][5],
		   StreamParams->H264SetGlobalParamPPS.ScalingList.NextTwoScalingList[i][6], StreamParams->H264SetGlobalParamPPS.ScalingList.NextTwoScalingList[i][7] );

    return CodecNoError;
}


// /////////////////////////////////////////////////////////////////////////
//
//      Function to dump out the decode
//      parameters from an mme command.
//

CodecStatus_t   Codec_MmeVideoH264_c::DumpDecodeParameters(            void    *Parameters )
{
unsigned int                      i;
unsigned int                      MaxDescriptor;
H264_TransformParam_fmw_t        *FrameParams;
H264_HostData_t                  *HostData;
H264_RefPictListAddress_t        *RefPicLists;

//

    FrameParams = (H264_TransformParam_fmw_t *)Parameters;
    HostData    = &FrameParams->HostData;
    RefPicLists = &FrameParams->InitialRefPictList;

    report( severity_info,  "AZA - FRAME PARAMS %08x\n", FrameParams );
    report( severity_info,  "AZA -       PictureStartAddrBinBuffer            = %08x\n", FrameParams->PictureStartAddrBinBuffer );
    report( severity_info,  "AZA -       PictureStopAddrBinBuffer             = %08x\n", FrameParams->PictureStopAddrBinBuffer );
    report( severity_info,  "AZA -       MaxSESBSize                          = %d bytes\n", FrameParams->MaxSESBSize );
    report( severity_info,  "AZA -       MainAuxEnable                        = %08x\n", FrameParams->MainAuxEnable );
    report( severity_info,  "AZA -       HorizontalDecimationFactor           = %08x\n", FrameParams->HorizontalDecimationFactor );
    report( severity_info,  "AZA -       VerticalDecimationFactor             = %08x\n", FrameParams->VerticalDecimationFactor );
    report( severity_info,  "AZA -       DecodingMode                         = %d\n", FrameParams->DecodingMode );
    report( severity_info,  "AZA -       AdditionalFlags                      = %08x\n", FrameParams->AdditionalFlags );
    report( severity_info,  "AZA -       Luma_p                               = %08x\n", FrameParams->DecodedBufferAddress.Luma_p );
    report( severity_info,  "AZA -       Chroma_p                             = %08x\n", FrameParams->DecodedBufferAddress.Chroma_p );
    report( severity_info,  "AZA -       MBStruct_p                           = %08x\n", FrameParams->DecodedBufferAddress.MBStruct_p );
    report( severity_info,  "AZA -       HostData\n" );
    report( severity_info,  "AZA -           PictureNumber                    = %d\n", HostData->PictureNumber );
    report( severity_info,  "AZA -           PicOrderCntTop                   = %d\n", HostData->PicOrderCntTop );
    report( severity_info,  "AZA -           PicOrderCntBot                   = %d\n", HostData->PicOrderCntBot );
    report( severity_info,  "AZA -           PicOrderCnt                      = %d\n", HostData->PicOrderCnt );
    report( severity_info,  "AZA -           DescriptorType                   = %d\n", HostData->DescriptorType );
    report( severity_info,  "AZA -           ReferenceType                    = %d\n", HostData->ReferenceType );
    report( severity_info,  "AZA -           IdrFlag                          = %d\n", HostData->IdrFlag );
    report( severity_info,  "AZA -           NonExistingFlag                  = %d\n", HostData->NonExistingFlag );
    report( severity_info,  "AZA -       InitialRefPictList\n" );

    report( severity_info,  "AZA -           ReferenceFrameAddresses\n" );
    report( severity_info,  "AZA -                 Luma    Chroma   MBStruct\n" );
    for( i=0; i<DecodeBufferCount; i++ )
    report( severity_info,  "AZA -               %08x %08x %08x\n",
		RefPicLists->ReferenceFrameAddress[i].DecodedBufferAddress.Luma_p,
		RefPicLists->ReferenceFrameAddress[i].DecodedBufferAddress.Chroma_p,
		RefPicLists->ReferenceFrameAddress[i].DecodedBufferAddress.MBStruct_p );

    MaxDescriptor       = 0;
    report( severity_info,  "AZA -           InitialPList0 Descriptor Indices\n" );
    report( severity_info,  "AZA -               " );
    for( i=0; i<RefPicLists->InitialPList0.RefPiclistSize; i++ )
    {
	report( severity_info, "%02d  ", RefPicLists->InitialPList0.RefPicList[i] );
	MaxDescriptor   = max( MaxDescriptor, (RefPicLists->InitialPList0.RefPicList[i]+1) );
    }
    report( severity_info,  "\n" );

    report( severity_info,  "AZA -           InitialBList0 Descriptor Indices\n" );
    report( severity_info,  "AZA -               " );
    for( i=0; i<RefPicLists->InitialBList0.RefPiclistSize; i++ )
    {
	report( severity_info, "%02d  ", RefPicLists->InitialBList0.RefPicList[i] );
	MaxDescriptor   = max( MaxDescriptor, (RefPicLists->InitialBList0.RefPicList[i]+1) );
    }
    report( severity_info,  "\n" );

    report( severity_info,  "AZA -           InitialBList1 Descriptor Indices\n" );
    report( severity_info,  "AZA -               " );
    for( i=0; i<RefPicLists->InitialBList1.RefPiclistSize; i++ )
    {
	report( severity_info, "%02d  ", RefPicLists->InitialBList1.RefPicList[i] );
	MaxDescriptor   = max( MaxDescriptor, (RefPicLists->InitialBList1.RefPicList[i]+1) );
    }
    report( severity_info,  "\n" );

    report( severity_info,  "AZA -           Picture descriptors (ReferenceDescriptorsBitsField = %08x)\n", RefPicLists->ReferenceDescriptorsBitsField );
    for( i=0; i<MaxDescriptor; i++ )
	report( severity_info, "AZA -               Desc %d - Buff %2d, PN = %2d, Cnt T %2d B %2d X %2d, DT = %d, RT = %d, IDR = %d, NEF = %d\n",
		i, RefPicLists->PictureDescriptor[i].FrameIndex,
		RefPicLists->PictureDescriptor[i].HostData.PictureNumber,
		RefPicLists->PictureDescriptor[i].HostData.PicOrderCntTop,
		RefPicLists->PictureDescriptor[i].HostData.PicOrderCntBot,
		RefPicLists->PictureDescriptor[i].HostData.PicOrderCnt,
		RefPicLists->PictureDescriptor[i].HostData.DescriptorType,
		RefPicLists->PictureDescriptor[i].HostData.ReferenceType,
		RefPicLists->PictureDescriptor[i].HostData.IdrFlag,
		RefPicLists->PictureDescriptor[i].HostData.NonExistingFlag );

//

    return CodecNoError;
}


// /////////////////////////////////////////////////////////////////////////
//
//      A local release reference frame, the external one inserts into our
//	processing list, and the parent class one is generic, this version
//	of the function releases the reference list slot allocation, then
//	passes control to the generic function.
//

CodecStatus_t   Codec_MmeVideoH264_c::H264ReleaseReferenceFrame( unsigned int	 ReferenceFrameDecodeIndex )
{
CodecStatus_t	Status;
unsigned int    Index;

    //
    // Release the appropriate entries in the reference list slot array
    //

    if( ReferenceFrameDecodeIndex == CODEC_RELEASE_ALL )
    {
	memset( ReferenceFrameSlotUsed, 0x00, H264_MAX_REFERENCE_FRAMES * sizeof(bool) );
	OutstandingSlotAllocationRequest	= INVALID_INDEX;
    }
    else
    {
	Status	= TranslateDecodeIndex( ReferenceFrameDecodeIndex, &Index );
	if( (Status == CodecNoError) && (BufferState[Index].ReferenceFrameCount == 1) )
	    ReferenceFrameSlotUsed[BufferState[Index].ReferenceFrameSlot]	= false;
    }

    //
    // Can we satisfy an outstanding reference frame slot request
    //

    ReferenceFrameSlotAllocate( INVALID_INDEX );

    //
    // And pass on down the generic chain
    //

    return Codec_MmeVideo_c::ReleaseReferenceFrame( ReferenceFrameDecodeIndex );
}


// /////////////////////////////////////////////////////////////////////////
//
//      A function to handle allocation of reference frame slots.
//	Split out since it can be called on a decode where the
//	allocation may be deferred, and on release where a deferred
//	allocation may be enacted. Only one allocation may be deferred
//	at any one time.
//

CodecStatus_t   Codec_MmeVideoH264_c::ReferenceFrameSlotAllocate( unsigned int		 BufferIndex )
{
unsigned int	i;

    //
    // Is there any outstanding request (that is still outstanding).
    //

    if( OutstandingSlotAllocationRequest != INVALID_INDEX )
    {
	if( BufferState[OutstandingSlotAllocationRequest].ReferenceFrameCount != 0 )
	{
	    for( i=0; (i<H264_MAX_REFERENCE_FRAMES) && ReferenceFrameSlotUsed[i]; i++ );

	    // Did we find one, if not just fall through
	    if( i < H264_MAX_REFERENCE_FRAMES )
	    {
		ReferenceFrameSlotUsed[i]						= true;
		BufferState[OutstandingSlotAllocationRequest].ReferenceFrameSlot	= i;
		OutstandingSlotAllocationRequest					= INVALID_INDEX;
	    }
	}
	else
	{
	    OutstandingSlotAllocationRequest	= INVALID_INDEX;
	}
    }

    //
    // Do we have a new request
    //

    if( BufferIndex != INVALID_INDEX )
    {
	//
	// If there is an outstanding request we have a problem
	//

	if( OutstandingSlotAllocationRequest != INVALID_INDEX )
	{
	    report( severity_error, "Codec_MmeVideoH264_c::ReferenceFrameSlotAllocate - Outstanding request when new request is received.\n" );
	    return CodecError;
	}

	//
	// Attempt to allocate, should we fail then make the request outstanding
	//

	for( i=0; (i<H264_MAX_REFERENCE_FRAMES) && ReferenceFrameSlotUsed[i]; i++ );

	if( i < H264_MAX_REFERENCE_FRAMES )
	{
	    ReferenceFrameSlotUsed[i]			= true;
	    BufferState[BufferIndex].ReferenceFrameSlot	= i;
	}
	else
	{
	    BufferState[BufferIndex].ReferenceFrameSlot = INVALID_INDEX;
	    OutstandingSlotAllocationRequest		= BufferIndex;
	}
    }

//

    return CodecNoError;
}


// /////////////////////////////////////////////////////////////////////////
//
//      The intermediate process, taking output from the pre-processor
//      and feeding it to the lower level of the codec.
//

OS_TaskEntry(Codec_MmeVideoH264_IntermediateProcess)
{
Codec_MmeVideoH264_c    *Codec = (Codec_MmeVideoH264_c *)Parameter;

    Codec->IntermediateProcess();

    OS_TerminateThread();
    return NULL;
}


#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)

#define DVB_SPEED_NORMAL_PLAY           1000
#define DVB_SPEED_STOPPED               0
#define DVB_SPEED_REVERSE_STOPPED       0x80000000


#define PLAY_SPEED_NORMAL_PLAY          DVB_SPEED_NORMAL_PLAY
#define PLAY_SPEED_STOPPED              DVB_SPEED_STOPPED
#define PLAY_SPEED_REVERSE_STOPPED      DVB_SPEED_REVERSE_STOPPED

// ----------------------

int Codec_MmeVideoH264_c::getPlayerSpeed(void)
{
    Rational_t		  Speed;
    PlayDirection_t	  Direction;
    int playerSpeed = 0;

    Player->GetPlaybackSpeed( Playback, &Speed, &Direction );

    playerSpeed = (int)IntegerPart (Speed * PLAY_SPEED_NORMAL_PLAY);
    if ((playerSpeed == PLAY_SPEED_STOPPED) && (Direction != PlayForward))
        playerSpeed = PLAY_SPEED_REVERSE_STOPPED;
    else if (Direction != PlayForward)
        playerSpeed = -playerSpeed;

    return playerSpeed;
}
#endif ///CONFIG_SH_QBOXHD_1_0

// ----------------------

void Codec_MmeVideoH264_c::IntermediateProcess( void )
{
h264pp_status_t PPStatus;
RingStatus_t    RingStatus;
unsigned int    Entry;
unsigned int    DecodeFrameIndex;
unsigned int    PPEntry;
unsigned int    PPSize;
unsigned int    PPStatusMask;
unsigned char	AllowBadPPFrames;

    //
    // Signal we have started
    //

#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
    OS_LockMutex(&HALT_Lock);
#endif ///CONFIG_SH_QBOXHD_1_0

    ProcessRunningCount++;
    OS_SetEvent( &StartStopEvent );

    //
    // Main loop
    //

    while( !Terminating )
    {
	//
	// Anything coming through the ring
	//

	RingStatus      = FramesInPreprocessorChainRing->Extract( &Entry, MAX_EVENT_WAIT );
	if( RingStatus == RingNothingToGet )
#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
        {
            if (HALT_Flag)
            {
     //This branch avoids possible deadlock during zapping
                OS_UnLockMutex(&HALT_Lock);
                while( !Terminating ) OS_SleepMilliSeconds( 100 );
            }
            else
            {
     //If there is no message to process, than it clears all decode buffers older than 5 secs.
                unsigned int i, Count;

                OS_LockMutex( &Lock );
                for(i=0;i<32;i++)
                {
                    if(BufferState[i].Buffer!=0)
                    {
                        BufferState[i].Buffer->GetOwnerCount(&Count);
                        if ((Count==1) && (BufferState[i].ReferenceFrameCount_time+5000<=OS_GetTimeInMilliSeconds()))
                        {
                            report( severity_error, "REMOVE buffer due to timeout 5 secs ----22222---- idx:%d  owncount: %d  refcount: %d  RefFrame: %d  reftime: %d   now: %d\n", i, Count, BufferState[i].ReferenceFrameCount, BufferState[i].ReferenceFrame_flag, BufferState[i].ReferenceFrameCount_time, OS_GetTimeInMilliSeconds() );
                            OS_UnLockMutex( &Lock );
                            Codec_MmeBase_c::DecrementReferenceCount(i);
                            OS_LockMutex( &Lock );
                        }
                    }
                }
                OS_UnLockMutex( &Lock );
            }
#endif ///CONFIG_SH_QBOXHD_1_0
	    continue;
#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
        }
#endif ///CONFIG_SH_QBOXHD_1_0

	//
	// Is frame aborted
	//

	if( FramesInPreprocessorChain[Entry].AbortedFrame || Terminating )
	    FramesInPreprocessorChain[Entry].Action     = ActionNull;

	//
	// Process activity - note aborted activity differs in consequences for each action.
	//

	switch( FramesInPreprocessorChain[Entry].Action )
	{
	    case ActionNull:
		break;

	    case ActionCallOutputPartialDecodeBuffers:
		Codec_MmeVideo_c::OutputPartialDecodeBuffers();
		break;

	    case ActionCallDiscardQueuedDecodes:
		DiscardFramesInPreprocessorChain        = false;
		Codec_MmeVideo_c::DiscardQueuedDecodes();
		break;

	    case ActionCallReleaseReferenceFrame:
		H264ReleaseReferenceFrame( FramesInPreprocessorChain[Entry].DecodeFrameIndex );
		break;

	    case ActionStashReleaseReferenceFrame:
		ReleaseReferenceFrameStash->Insert( FramesInPreprocessorChain[Entry].DecodeFrameIndex );
		break;

	    case ActionPassOnPreProcessedFrame:
		PPStatus        = H264ppGetPreProcessedBuffer( PreProcessorDevice, &PPEntry, &PPSize, &PPStatusMask );
		if( PPStatus != h264pp_ok )
		{
		    report( severity_error, "Codec_MmeVideoH264_c::IntermediateProcess - Failed to retrieve a buffer from the pre-processor.\n" );
		    break;
		}

		if( PPStatusMask != 0 )
		{
		    AllowBadPPFrames	= Player->PolicyValue( Playback, Stream, PolicyH264AllowBadPreProcessedFrames );
		    if( FramesInPreprocessorChain[Entry].SpeculativePreProcess ||
			(AllowBadPPFrames != PolicyValueApply) )
		    {
			report( severity_error, "Codec_MmeVideoH264_c::IntermediateProcess - Pre-processing failed.\n" );
		    	break;
		    }
		}

		if( PPEntry != Entry )
		{
		    report( severity_error, "Codec_MmeVideoH264_c::IntermediateProcess - Retrieved wrong identifier from pre-processor (%d %d).\n", PPEntry, Entry );
		    break;
		}

		if( DiscardFramesInPreprocessorChain )
		    break;

		FramesInPreprocessorChain[Entry].PreProcessorBuffer->SetUsedDataSize( PPSize );

	    case ActionPassOnFrame:
		if( Terminating )
		    break;

#if defined(CONFIG_SH_QBOXHD_1_0) || defined(CONFIG_SH_QBOXHD_MINI_1_0)
    //H264 deadlock work around
    //It checks decoder buffers before input CodedBuffer to Video Codec. If all buffers are full, then waits for at least one empties.
    //After 1.5 secs clears the oldest one. Besides it clears all buffers older than 5 secs.

                {
                    unsigned int i, ii, c, k, tttime, time=0, Count, mintime;
                    unsigned int NUM_BUFFERS;


                    ((class Manifestor_Base_c*)Manifestor)->GetDecodeBufferCount( &NUM_BUFFERS );

					do
					{
						c=0;
						for(i=0;i<32;i++) if(BufferState[i].Buffer!=0) c++;
						if (!Terminating && (getPlayerSpeed()==PLAY_SPEED_STOPPED) && c>=NUM_BUFFERS)
						{
							OS_SleepMilliSeconds( 100 );
						}
					} while(!Terminating && (getPlayerSpeed()==PLAY_SPEED_STOPPED) && c>=NUM_BUFFERS);


                    if (c>=NUM_BUFFERS)
                    {
                        unsigned char buftorelease[32];
                        unsigned char buftorelease_cnt = 0;
                        CodecStatus_t	Status;
                        unsigned int    Index;

                        time = tttime = OS_GetTimeInMilliSeconds();

                        while(1)
                        {
                            OS_SleepMilliSeconds( 5 );

                            buftorelease_cnt = 0;
                            for( i=0; i<H264_CODED_FRAME_COUNT; i++ )
                            {
                                if( FramesInPreprocessorChain[i].Used && ( FramesInPreprocessorChain[i].Action == ActionCallReleaseReferenceFrame ||
                                                                           FramesInPreprocessorChain[i].Action == ActionStashReleaseReferenceFrame   ) )
                                {
                                    Status	= TranslateDecodeIndex( FramesInPreprocessorChain[i].DecodeFrameIndex, &Index );
                                    if (Status == CodecNoError) buftorelease[buftorelease_cnt++] = Index;
                                }
                            }

                            ReleaseReferenceFrameStash->Insert(0xBEEFDEAD);
                            do
                            {
                                ReleaseReferenceFrameStash->Extract( &DecodeFrameIndex );
                                if (DecodeFrameIndex == 0xBEEFDEAD) break;
                                ReleaseReferenceFrameStash->Insert( DecodeFrameIndex );
                                Status = TranslateDecodeIndex( DecodeFrameIndex, &Index );
                                if (Status == CodecNoError) buftorelease[buftorelease_cnt++] = Index;
                            } while(1);


                            c=0;

                            OS_LockMutex( &Lock );
                            for(i=0;i<32;i++)
                            {
                                if(BufferState[i].Buffer!=0)
                                {
                                    c++;

                                    {
                                        BufferState[i].Buffer->GetOwnerCount(&Count);
                                        if ((Count==1) && (BufferState[i].ReferenceFrameCount_time+5000<=OS_GetTimeInMilliSeconds()))
                                        {
                                            for( ii=0; ii<buftorelease_cnt; ii++ )     if (buftorelease[ii] == i) break;
                                            if (ii == buftorelease_cnt)
                                            {
                                                report( severity_error, "REMOVE buffer due to timeout 5 secs ----11111---- idx:%d  owncount: %d  refcount: %d  RefFrame: %d  reftime: %d   now: %d\n", i, Count, BufferState[i].ReferenceFrameCount, BufferState[i].ReferenceFrame_flag, BufferState[i].ReferenceFrameCount_time, OS_GetTimeInMilliSeconds() );
                                                OS_UnLockMutex( &Lock );
                                                Codec_MmeBase_c::DecrementReferenceCount(i);
                                                OS_LockMutex( &Lock );
                                                c--;
                                            }
                                        }
                                    }
                                }
                            }
                            OS_UnLockMutex( &Lock );

                            if (c<NUM_BUFFERS) break;

                            if (time+1500<=OS_GetTimeInMilliSeconds())
                            {
                                mintime = 0;
                                c=0;
                                OS_LockMutex( &Lock );
                                for(i=0;i<32;i++)
                                {
                                    if(BufferState[i].Buffer!=0)
                                    {
                                        BufferState[i].Buffer->GetOwnerCount(&Count);
                                        if ((Count==1) && (BufferState[i].ReferenceFrame_flag || BufferState[i].ReferenceFrameCount_time+5000<=OS_GetTimeInMilliSeconds()) && (BufferState[i].ReferenceFrameCount_time<mintime || mintime==0))
                                        {
                                            for( ii=0; ii<buftorelease_cnt; ii++ )     if (buftorelease[ii] == i) break;
                                            if (ii == buftorelease_cnt)
                                            {
                                                mintime = BufferState[i].ReferenceFrameCount_time;
                                                k = i;
                                            }
                                        }
                                    }
                                }
                                if (mintime != 0)
                                {
                                    BufferState[k].Buffer->GetOwnerCount(&Count);
                                    report( severity_error, "REMOVE oldest buffer ------------- idx:%d  owncount: %d  refcount: %d  RefFrame: %d  reftime: %d   now: %d\n", k, Count, BufferState[k].ReferenceFrameCount, BufferState[k].ReferenceFrame_flag, BufferState[k].ReferenceFrameCount_time, OS_GetTimeInMilliSeconds() );
                                    OS_UnLockMutex( &Lock );
                                    Codec_MmeBase_c::DecrementReferenceCount(k);
                                    OS_LockMutex( &Lock );
                                }
                                OS_UnLockMutex( &Lock );
                            }


							do
							{
								c=0;
								for(i=0;i<32;i++) if(BufferState[i].Buffer!=0) c++;
								if (!Terminating && (getPlayerSpeed()==PLAY_SPEED_STOPPED) && c>=NUM_BUFFERS)
								{
									OS_SleepMilliSeconds( 100 );
								}
							} while(!Terminating && (getPlayerSpeed()==PLAY_SPEED_STOPPED) && c>=NUM_BUFFERS);


                            if (c<NUM_BUFFERS) break;

                            if (tttime+1000<=OS_GetTimeInMilliSeconds())
                            {
                                //report( severity_error, "******************************************************************\n");
                                OS_LockMutex( &Lock );
                                for(i=0;i<32;i++)
                                {
                                    if(BufferState[i].Buffer!=0)
                                    {
                                        BufferState[i].Buffer->GetOwnerCount(&Count);
                                        //report( severity_error, "***************************************** idx:%d  owncount: %d  refcount: %d  RefFrame: %d  reftime: %d  now: %d\n", i, Count, BufferState[i].ReferenceFrameCount, BufferState[i].ReferenceFrame_flag, BufferState[i].ReferenceFrameCount_time, OS_GetTimeInMilliSeconds() );
                                    }
                                }
                                OS_UnLockMutex( &Lock );
                                //report( severity_error, "==================================================================\n");
                                //for( i=0; i<buftorelease_cnt; i++ )
                                //{
                                //    report( severity_error, "========================================= idx:%d \n", buftorelease[i]);
                                //}
                                //report( severity_error, "******************************************************************\n");
                                tttime = OS_GetTimeInMilliSeconds();
                            }
                        }
                    }
            }
#endif ///CONFIG_SH_QBOXHD_1_0

		Codec_MmeVideo_c::Input( FramesInPreprocessorChain[Entry].CodedBuffer );
		break;

	}

	//
	// Is this a suitable point at wish to perform the stash of reference frame releases
	//

	if( FramesInPreprocessorChain[Entry].EmptyReleaseReferenceFrameStash )
	{
	    while( ReleaseReferenceFrameStash->NonEmpty() )
	    {
		ReleaseReferenceFrameStash->Extract( &DecodeFrameIndex );
		H264ReleaseReferenceFrame( DecodeFrameIndex );
	    }
	}

	//
	// Release this entry
	//

	if( FramesInPreprocessorChain[Entry].CodedBuffer != NULL )
	    FramesInPreprocessorChain[Entry].CodedBuffer->DecrementReferenceCount( IdentifierH264Intermediate );

	FramesInPreprocessorChain[Entry].Used   = false;
    }

    //
    // Clean up the ring
    //

    while( FramesInPreprocessorChainRing->NonEmpty() )
    {
	FramesInPreprocessorChainRing->Extract( &Entry );

	if( FramesInPreprocessorChain[Entry].CodedBuffer != NULL )
	    FramesInPreprocessorChain[Entry].CodedBuffer->DecrementReferenceCount( IdentifierH264Intermediate );

	FramesInPreprocessorChain[Entry].Used   = false;
    }

    //
    // Signal we have terminated
    //

    ProcessRunningCount--;
    OS_SetEvent( &StartStopEvent );
}


