/*
Copyright 2018 Jay Sorg

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#if defined(HAVE_CONFIG_H)
#include "config_ac.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* this should be before all X11 .h files */
#include <xorg-server.h>
#include <xorgVersion.h>

/* all driver need this */
#include <xf86.h>
#include <xf86_OSproc.h>

#include <mipict.h>
#include <picture.h>

#include "rdp.h"
#include "rdpDraw.h"
#include "rdpClientCon.h"
#include "rdpReg.h"
#include "rdpTriangles.h"

/******************************************************************************/
#define LOG_LEVEL 1
#define LLOGLN(_level, _args) \
    do { if (_level < LOG_LEVEL) { ErrorF _args ; ErrorF("\n"); } } while (0)

/******************************************************************************/
static void
rdpTrianglesOrg(PictureScreenPtr ps, rdpPtr dev, CARD8 op,
                PicturePtr pSrc, PicturePtr pDst,
                PictFormatPtr maskFormat, INT16 xSrc, INT16 ySrc,
                int ntris, xTriangle *tris)
{
    ps->Triangles = dev->Triangles;
    ps->Triangles(op, pSrc, pDst, maskFormat, xSrc, ySrc, ntris, tris);
    ps->Triangles = rdpTriangles;
}

/******************************************************************************/
void
rdpTriangles(CARD8 op, PicturePtr pSrc, PicturePtr pDst,
             PictFormatPtr maskFormat, INT16 xSrc, INT16 ySrc,
             int ntris, xTriangle *tris)
{
    ScreenPtr pScreen;
    rdpPtr dev;
    PictureScreenPtr ps;
    BoxRec box;
    RegionRec reg;

    LLOGLN(10, ("rdpTriangles:"));
    pScreen = pDst->pDrawable->pScreen;
    dev = rdpGetDevFromScreen(pScreen);
    dev->counts.rdpTrianglesCallCount++;
    miTriangleBounds(ntris, tris, &box);
    box.x1 += pDst->pDrawable->x;
    box.y1 += pDst->pDrawable->y;
    box.x2 += pDst->pDrawable->x;
    box.y2 += pDst->pDrawable->y;
    rdpRegionInit(&reg, &box, 0);
    ps = GetPictureScreen(pScreen);
    if (pDst->pCompositeClip != NULL)
    {
        rdpRegionIntersect(&reg, pDst->pCompositeClip, &reg);
    }
    /* do original call */
    rdpTrianglesOrg(ps, dev, op, pSrc, pDst, maskFormat, xSrc, ySrc,
                    ntris, tris);
    rdpClientConAddAllReg(dev, &reg, pDst->pDrawable);
    rdpRegionUninit(&reg);
}
