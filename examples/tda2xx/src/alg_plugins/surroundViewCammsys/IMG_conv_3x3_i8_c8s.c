/* ======================================================================== *
 * IMGLIB -- TI Image and Video Processing Library                          *
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/   *
 *                                                                          *
 *                                                                          *
 *  Redistribution and use in source and binary forms, with or without      *
 *  modification, are permitted provided that the following conditions      *
 *  are met:                                                                *
 *                                                                          *
 *    Redistributions of source code must retain the above copyright        *
 *    notice, this list of conditions and the following disclaimer.         *
 *                                                                          *
 *    Redistributions in binary form must reproduce the above copyright     *
 *    notice, this list of conditions and the following disclaimer in the   *
 *    documentation and/or other materials provided with the                *
 *    distribution.                                                         *
 *                                                                          *
 *    Neither the name of Texas Instruments Incorporated nor the names of   *
 *    its contributors may be used to endorse or promote products derived   *
 *    from this software without specific prior written permission.         *
 *                                                                          *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS     *
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT       *
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR   *
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT    *
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,   *
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT        *
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,   *
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY   *
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT     *
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE   *
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.    *
 * ======================================================================== */
                                                                                          
/* ======================================================================== */
/*  NAME                                                                    */
/*      IMG_conv_3x3_i8_c8s                                                 */
/*                                                                          */
/*  USAGE                                                                   */
/*      This routine is C-callable and can be called as:                    */
/*                                                                          */
/*          void IMG_conv_3x3_i8_c8s                                        */
/*          (                                                               */
/*              const unsigned char  *restrict   imgin_ptr,                 */
/*                    unsigned char  *restrict  imgout_ptr,                 */
/*                             short                 width,                 */
/*                             short                 pitch,                 */
/*              const          char  *restrict    mask_ptr,                 */
/*                             short                 shift                  */
/*          )                                                               */
/*                                                                          */
/*      imgin_ptr :   pointer to an input  array of 8 bit pixels            */
/*      imgout_ptr:   pointer to an output array of 8 bit pixels            */
/*      pitch     :   No. of columns in the Input Image.                    */
/*      width     :   No. of output pixels to be calculated.                */
/*      mask_ptr  :   pointer to 8 bit filter mask.                         */
/*      shift     :   user specified shift amount                           */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      The convolution kernel accepts three rows of 'pitch' input pixels   */
/*      and produces one row of 'width' output pixels using the input       */
/*      mask of 3 by 3. This convolution performs a point by point mult-    */
/*      iplication of 3 by 3 mask with the input image. The result of 9     */
/*      multiplications are then summed together to produce a 32-bit conv-  */
/*      olution intermediate sum. The user defined shift value is used to   */
/*      shift this convolution sum, down to the byte range and store in an  */
/*      output array. The result being stored is also range limited between */
/*      0 to 255 and will be saturated accordingly. The mask is moved one   */
/*      column at a time, advancing the mask over the entire image until    */
/*      the entire 'width' is covered. The masks are provided as  8-bit     */
/*      signed values. The input image pixels are provided as 8-bit uns-    */
/*      igned pixels and the output pixels will be in 8-bit unsigned.       */
/*                                                                          */
/*      The natural C implementation has no restrictions. The Optimized     */
/*      and IntrinsiC codes has restrictions as noted in the ASSUMPTIONS    */
/*      below.                                                              */
/*                                                                          */
/*  ASSUMPTIONS                                                             */
/*      1. width:       'width' >= 4 and multiple of 4.                     */
/*      2. pitch:       'pitch' >= 'width'                                  */
/*      3. Output array should be word-aligned.                             */
/*      4. No restrictions on alignment on input data and mask              */
/*      5. Input and Output arrays should not overlap.                      */
/*                                                                          */
/*  COMPATIBILITY                                                           */
/*      This code is compatible with C66x                                   */
/*                                                                          */
/* ======================================================================== */

#pragma CODE_SECTION(IMG_conv_3x3_i8_c8s,   ".text:optimized");


void IMG_conv_3x3_i8_c8s
(
    const unsigned char *restrict  imgin_ptr,
    unsigned char       *restrict imgout_ptr,
    short                              width,
    short                              pitch,
    const char          *restrict   mask_ptr,
    short                              shift
)
{
  int                 i;
  int                 sum0,           sum1,           sum2;
  int                 sum3;
                                      
  unsigned int        mask1_0,        mask2_0,        mask3_0;
  unsigned int        mask1_1,        mask2_1,        mask3_1;
  unsigned int        mask_tmp;

  unsigned int        r1_7654,        r1_3210;
  unsigned int        r2_7654,        r2_3210;
  unsigned int        r3_7654,        r3_3210;
  unsigned int        r1_5432,        r2_5432,        r3_5432;

  unsigned int        sum32,          sum10,          sum3210;

  double              r1_76543210,    r2_76543210,    r3_76543210;

  const unsigned char	*in0,           *in1,           *in2;


  /* -------------------------------------------------------------------- */
  /*  Inform the compiler by _nasserts the following:                     */
  /*      a) The output array is word aligned                             */
  /*      b) Filter Mask array is double word aligned                     */
  /*      c) The width is greater than or equal to 6                      */
  /*      d) The width is a multiple of 4.                                */
  /* -------------------------------------------------------------------- */
  _nassert((int)imgout_ptr % 4 == 0);    /* word aligned                  */
  _nassert(width               >= 4);    /* width greater or equal to 4   */
  _nassert(width           % 4 == 0);    /* width is a multiple of 4      */

  /*----------------------------------------------------------------------*/
  /*  Read mask values and prepare registers for convolution              */
  /*  Read in reverse to account for mask rotation                        */
  /*----------------------------------------------------------------------*/
  mask_tmp = _mem4_const(&mask_ptr[6]);
  mask1_1  = _packlh2(_swap4(mask_tmp),_swap4(mask_tmp)) & 0xffffff00;

  mask_tmp = _mem4_const(&mask_ptr[3]);
  mask2_1  = _packlh2(_swap4(mask_tmp),_swap4(mask_tmp)) & 0xffffff00;

  mask_tmp = _mem4_const(&mask_ptr[0]);
  mask3_1  = _packlh2(_swap4(mask_tmp),_swap4(mask_tmp)) & 0xffffff00;

  mask1_0 = _extu(mask1_1, 0, 8);
  mask2_0 = _extu(mask2_1, 0, 8);
  mask3_0 = _extu(mask3_1, 0, 8);

  /* -------------------------------------------------------------------- */
  /*  Initialise pointers to the start of each row                        */
  /* -------------------------------------------------------------------- */
  in0     = imgin_ptr;
  in1     = in0 + pitch;
  in2     = in1 + pitch;

  /* -------------------------------------------------------------------- */
  /*  Loop is manually unrolled by 4                                      */
  /* -------------------------------------------------------------------- */
  #pragma MUST_ITERATE(1, ,1)

  for (i=0; i<width; i+=4) {
    /* ------------------------------------------------------------------ */
    /*  Load 8 values from each row (only use 6)                          */
    /* ------------------------------------------------------------------ */
    r1_76543210 = _memd8_const(in0);
    r2_76543210 = _memd8_const(in1);
    r3_76543210 = _memd8_const(in2);

    /* ------------------------------------------------------------------ */
    /*  Update pointers for next iteration                                */
    /* ------------------------------------------------------------------ */
    in0 += 4;
    in1 += 4;
    in2 += 4;

    /* ------------------------------------------------------------------ */
    /*  Extract lower four values for 1st set of convolution sums         */
    /* ------------------------------------------------------------------ */
    r1_3210 = _lo(r1_76543210);
    r2_3210 = _lo(r2_76543210);
    r3_3210 = _lo(r3_76543210);
    r1_7654 = _hi(r1_76543210);
    r2_7654 = _hi(r2_76543210);
    r3_7654 = _hi(r3_76543210);

    /* ------------------------------------------------------------------ */
    /*  Calculate 1st pair of sums and pack                               */
    /* ------------------------------------------------------------------ */
    sum0 = (_dotpsu4(mask1_0, r1_3210) + 
            _dotpsu4(mask2_0, r2_3210) + 
            _dotpsu4(mask3_0, r3_3210)) >> shift;

    sum1 = (_dotpsu4(mask1_1, r1_3210) + 
            _dotpsu4(mask2_1, r2_3210) + 
            _dotpsu4(mask3_1, r3_3210)) >> shift;

    sum10 = _spack2(sum1,sum0);

    /* ------------------------------------------------------------------ */
    /*  Combine middle four values for 2nd set of convolutions sums       */
    /* ------------------------------------------------------------------ */
    r1_5432 = _packlh2(r1_7654, r1_3210);
    r2_5432 = _packlh2(r2_7654, r2_3210);
    r3_5432 = _packlh2(r3_7654, r3_3210);

    /* ------------------------------------------------------------------ */
    /*  Calculate 2nd pair of sums and pack                               */
    /* ------------------------------------------------------------------ */
    sum2 = (_dotpsu4(mask1_0, r1_5432) + 
            _dotpsu4(mask2_0, r2_5432) + 
            _dotpsu4(mask3_0, r3_5432)) >> shift;

    sum3 = (_dotpsu4(mask1_1, r1_5432) + 
            _dotpsu4(mask2_1, r2_5432) + 
            _dotpsu4(mask3_1, r3_5432)) >> shift;

    sum32 = _spack2(sum3,sum2);

    /* ------------------------------------------------------------------ */
    /* Check for saturation and pack the 4 bytes to store using amem4     */
    /* ------------------------------------------------------------------ */
    sum3210 = _spacku4(sum32,sum10);

    _amem4((void*) &imgout_ptr[i]) = sum3210;
  }
}

/* ======================================================================== */
/*  End of file: IMG_conv_3x3_i8_c8s.c                                      */
/* ======================================================================== */
