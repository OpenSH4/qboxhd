/*
 * 	Copyright (C) 2010 Duolabs Srl
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/ioctl.h>
#include <linux/fb.h>
#include <fcntl.h>

#include <directfb.h>

#define DFBCHECK(x...) 						\
{ 								\
    DFBResult err = x; 						\
    								\
    if (err != DFB_OK) { 					\
        fprintf(stderr, "%s <%d>:\n\t", __FILE__, __LINE__ ); 	\
        DirectFBErrorFatal(#x, err);				\
    } 								\
}

int main(int argc, char **argv) {
	IDirectFB              *dfb = NULL;
	IDirectFBSurface       *surf = NULL;
	IDirectFBSurface       *img_surf = NULL;
    DFBSurfaceDescription  dsc, surf_dsc;
	IDirectFBImageProvider *img = NULL;
	int fb=0;
	struct fb_var_screeninfo var_screeninfo;

	printf("Version of df_showimage: 0.0.1\n");

	if (argc != 2) {
		fprintf(stderr, "\nUsage:\t%s <filename>\n\n", argv[0]);
		return -1;
	}
	/* Onbtain the resolution */
	fb = open("/dev/fb0", O_RDWR);
	ioctl(fb, FBIOGET_VSCREENINFO, &var_screeninfo);
	close(fb);


	/* Init DFB */
    DFBCHECK(DirectFBInit(&argc, &argv));
    DFBCHECK(DirectFBCreate(&dfb));
	DFBCHECK(dfb->SetCooperativeLevel(dfb, DFSCL_FULLSCREEN));
	/*DFBCHECK(dfb->SetCooperativeLevel(dfb, DLSCL_SHARED));*/

	dsc.flags = DSDESC_CAPS|DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT;
	dsc.caps = DSCAPS_PRIMARY;
	dsc.width = var_screeninfo.xres;
	dsc.height = var_screeninfo.yres;
	dsc.pixelformat=DSPF_ARGB;

    DFBCHECK(dfb->CreateSurface(dfb, &dsc, &surf));

	/* Create background image */
    DFBCHECK(dfb->CreateImageProvider(dfb, argv[1], &img));
    DFBCHECK(img->GetSurfaceDescription(img, &surf_dsc));
    DFBCHECK(dfb->CreateSurface(dfb, &surf_dsc, &img_surf));
    DFBCHECK(img->RenderTo(img, img_surf, NULL));
    DFBCHECK(img->Release(img));

	DFBCHECK(surf->StretchBlit(surf, img_surf, NULL, NULL));

	/* FIXME: Safe value for now, but it's not precise */
	sleep(4);

    img_surf->Release(img_surf);
    surf->Release(surf);
    dfb->Release(dfb);

    return 0;
}
