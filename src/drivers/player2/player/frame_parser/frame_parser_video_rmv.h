/************************************************************************
COPYRIGHT (C) STMicroelectronics R&D Ltd 2007

Source file name : frame_parser_video_rmv.h
Author :           Mark C

Definition of the frame parser video Real Media Video class implementation for player 2.


Date        Modification                                    Name
----        ------------                                    --------
24-Oct-08   Created                                         Julian

************************************************************************/

#ifndef H_FRAME_PARSER_VIDEO_RMV
#define H_FRAME_PARSER_VIDEO_RMV

// /////////////////////////////////////////////////////////////////////
//
//      Include any component headers

#include "rmv.h"
#include "frame_parser_video.h"


// /////////////////////////////////////////////////////////////////////////
//
// Locally defined constants
//


// /////////////////////////////////////////////////////////////////////////
//
// Locally defined structures
//

// /////////////////////////////////////////////////////////////////////////
//
// The class definition
//

/// Frame parser for RMV video.
class FrameParser_VideoRmv_c : public FrameParser_Video_c
{
private:

    // Data

    RmvStreamParameters_t      *StreamParameters;
    RmvFrameParameters_t       *FrameParameters;

    RmvStreamParameters_t       CopyOfStreamParameters;

    bool                        StreamFormatInfoValid;
    Rational_t                  FrameRate;

    unsigned int                LastTemporalReference;
    unsigned int                TemporalReferenceBase;
    unsigned long long          FramePTS;
    unsigned int                FrameNumber;
#if defined (RECALCULATE_FRAMERATE)
    unsigned int                InitialTemporalReference;
    unsigned int                PCount;
    Rational_t                  CalculatedFrameRate;
#endif

    // Functions

    FrameParserStatus_t         ReadStreamFormatInfo(                           void );
    FrameParserStatus_t         ReadPictureHeader(                              void );

    bool                        NewStreamParametersCheck(                       void );
    FrameParserStatus_t         CommitFrameForDecode(                           void );


public:

    //
    // Constructor function
    //

    FrameParser_VideoRmv_c( void );
    ~FrameParser_VideoRmv_c( void );

    //
    // Overrides for component base class functions
    //

    FrameParserStatus_t   Reset(                                                void );

    //
    // FrameParser class functions
    //

    FrameParserStatus_t         RegisterOutputBufferRing(                       Ring_t          Ring );

    //
    // Stream specific functions
    //

    FrameParserStatus_t   ReadHeaders(                                          void );

    FrameParserStatus_t   ResetReferenceFrameList(                              void );
    FrameParserStatus_t   PrepareReferenceFrameList(                            void );

    FrameParserStatus_t   ForPlayUpdateReferenceFrameList(                      void );

    FrameParserStatus_t   RevPlayProcessDecodeStacks(                           void );
    FrameParserStatus_t   RevPlayGeneratePostDecodeParameterSettings(           void );
    FrameParserStatus_t   RevPlayRemoveReferenceFrameFromList(                  void );
};

#endif

