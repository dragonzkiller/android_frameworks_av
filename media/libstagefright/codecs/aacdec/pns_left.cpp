/* ------------------------------------------------------------------
 * Copyright (C) 1998-2009 PacketVideo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */
/*
 Pathname: pns_left.c

------------------------------------------------------------------------------
 REVISION HISTORY

 Description:  Modified from original shareware code

 Description:  Brought code in-line with PV standards.
               Merged PNS and Intensity blocks into one function.
               Modified routine to interface with a fixed-point implementation.
               Modified variable names for clarity.
               Improved for-loop structure that was previously checking
               the codebook used in each scale factor band multiple times.

 Description:  Added some comments for clarity.

 Description:  Changed strategy for q-format.  Q-format for SFBs should not
 be grouped.

 Description: Function had to be modified to obey new interpretation of the
 sfb_prediction_used flag.  LTP takes precedence, and PNS should not be
 executed when a collision occurs between these two tools.

 Description:
 (1) Added flag "ltp_data_present"
 (2) Where feasible, I updated the code to resemble right_ch_sfb_tools_ms.c
     Just for conformance, readability.

 Description: Added include file - "e_huffmanconst.h"

 Description: The same "Factors" pointer indexing problem that existed in
 right_ch_sfb_tools_ms also existed here in pns_left.c

 Description:  Modified how groups and windows are handled, as the multigroup
 case was not correct

 Who:                       Date:
 Description:
------------------------------------------------------------------------------
 INPUT AND OUTPUT DEFINITIONS

 Inputs:

    const FrameInfo *pFrameInfo         = Pointer to structure that holds
                                          information about each group.
                                          (long block flag,
                                           number of windows,
                                           scalefactor bands per group, etc.)

    const Int        group[]            = Array that contains indexes of the
                                          the first window in the next group.

    const Int        codebook_map[]     = Array that denotes which Huffman
                                          codebook was used for the encoding
                                          of each scalefactor band.

    const Int        factors[]          = Array of scalefactors

    Int              sfb_prediction_used[] = Flag that denotes the activation
                                             of long term prediction on a sfb
                                             basis.

    Bool             ltp_data_present   = Flag that denotes whether LTP
                                          is active for the entire frame.  If
                                          this flag is FALSE,
                                          sfb_prediction_used is garbage.

    Int32            spectral_coef[]    = Array of pointers pointing to the
                                          spectral coef's for the LEFT channel.

    Int              q_format[]           = Q-format for the information
                                            pointed to by spectral_coef.
                                            Indexed by scalefactor band.

    Int32           *pCurrentSeed         = Pointer to the current seed for the
                                            random number generator.
                                            (gen_rand_vector)

 Local Stores/Buffers/Pointers Needed:

 Global Stores/Buffers/Pointers Needed:

 Outputs:

 Pointers and Buffers Modified:
    Int32  spectral_coef[]    =   Contains the new spectral information

 Local Stores Modified:

 Global Stores Modified:

------------------------------------------------------------------------------
 FUNCTION DESCRIPTION

 The function steps through each scalefactor band in the group, and
 checks for the use of Huffman codebook NOISE_HCB.

 When a SFB utilizing NOISE_HCB is detected, the band in every window in the
 group has its spectral information filled with scaled random data.

 The scaled random data is generated by the function gen_rand_vector.

------------------------------------------------------------------------------
 REQUIREMENTS

 This module shall replace bands that were encoded with the Huffman codebook
 NOISE_HCB with random noise as returned from gen_rand_vector().  The result
 shall be perceptually indistinguishable from the result obtained by the
 ISO decoder.

------------------------------------------------------------------------------
 REFERENCES

 (1) ISO/IEC 14496-3:1999(E)
     Part 3
        Subpart 4.5.5   Figures
        Subpart 4.6.2   ScaleFactors
        Subpart 4.6.12  Perceptual Noise Substituion (PNS)

 (2) MPEG-2 NBC Audio Decoder
   "This software module was originally developed by AT&T, Dolby
   Laboratories, Fraunhofer Gesellschaft IIS in the course of development
   of the MPEG-2 NBC/MPEG-4 Audio standard ISO/IEC 13818-7, 14496-1,2 and
   3. This software module is an implementation of a part of one or more
   MPEG-2 NBC/MPEG-4 Audio tools as specified by the MPEG-2 NBC/MPEG-4
   Audio standard. ISO/IEC  gives users of the MPEG-2 NBC/MPEG-4 Audio
   standards free license to this software module or modifications thereof
   for use in hardware or software products claiming conformance to the
   MPEG-2 NBC/MPEG-4 Audio  standards. Those intending to use this software
   module in hardware or software products are advised that this use may
   infringe existing patents. The original developer of this software
   module and his/her company, the subsequent editors and their companies,
   and ISO/IEC have no liability for use of this software module or
   modifications thereof in an implementation. Copyright is not released
   for non MPEG-2 NBC/MPEG-4 Audio conforming products.The original
   developer retains full right to use the code for his/her own purpose,
   assign or donate the code to a third party and to inhibit third party
   from using the code for non MPEG-2 NBC/MPEG-4 Audio conforming products.
   This copyright notice must be included in all copies or derivative
   works."
   Copyright(c)1996.

------------------------------------------------------------------------------
 PSEUDO-CODE

    pFirst_Window_Coefs = spectral_coef;

    window_start = 0;

    tot_sfb = 0;

    DO

        num_bands = pFrameInfo->sfb_per_win[window_start];
        pBand     = pFrameInfo->win_sfb_top[window_start];

        partition = *(pGroup);
        pGroup = pGroup + 1;

        band_start = 0;

        coef_per_win = pFrameInfo->coef_per_win[window_start];

        wins_in_group = (partition - window_start);

        FOR (sfb = num_bands; sfb > 0; sfb--)

            band_stop = *pBand;
            pBand = pBand + 1;

            IF (*(pCodebookMap++) == NOISE_HCB )

                tempInt = sfb_prediction_used[tot_sfb] AND ltp_data_present;

                IF (tempInt == FALSE)

                    pWindow_Coef = pFirst_Window_Coefs + band_start;

                    band_length = (band_stop - band_start);

                    start_indx = tot_sfb;

                    tempInt = *(pFactors);

                    FOR (win_indx = wins_in_group; win_indx > 0; win_indx--)

                        CALL gen_rand_vector( pWindow_CoefR,
                                              band_length,
                                              pCurrentSeed,
                                              tempInt);

                        MODIFYING pWindow_CoefR = scaled random noise
                                  pCurrentSeed  = current state of the random
                                                  noise generator.

                        RETURNING q_format[start_indx] = q-format for this sfb.

                        pWindow_Coef = pWindow_Coef + coef_per_win;

                        start_indx = start_indx +
                                     pFrameInfo->sfb_per_win[win_indx];

                    ENDFOR

                ENDIF

            ENDIF

            band_start = band_stop;

            tot_sfb = tot_sfb + 1;

            pFactors = pFactors + 1;

        ENDFOR

        coef_per_win = coef_per_win * wins_in_group;
        wins_in_group = wins_in_group - 1;

        tot_sfb = tot_sfb + num_bands * wins_in_group;
        pFactors = pFactors + num_bands * wins_in_group;

        pFirst_Window_Coefs = pFirst_Window_Coefs + coef_per_win;

        window_start = partition;

    WHILE (partition < pFrameInfo->num_win);

------------------------------------------------------------------------------
 RESOURCES USED
   When the code is written for a specific target processor the
     the resources used should be documented below.

 STACK USAGE: [stack count for this module] + [variable to represent
          stack usage for each subroutine called]

     where: [stack usage variable] = stack usage for [subroutine
         name] (see [filename].ext)

 DATA MEMORY USED: x words

 PROGRAM MEMORY USED: x words

 CLOCK CYCLES: [cycle count equation for this module] + [variable
           used to represent cycle count for each subroutine
           called]

     where: [cycle count variable] = cycle count for [subroutine
        name] (see [filename].ext)

------------------------------------------------------------------------------
*/


/*----------------------------------------------------------------------------
; INCLUDES
----------------------------------------------------------------------------*/
#include "pv_audio_type_defs.h"
#include "pns_left.h"
#include "e_huffmanconst.h"
#include "gen_rand_vector.h"

/*----------------------------------------------------------------------------
; MACROS
; Define module specific macros here
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
; DEFINES
; Include all pre-processor statements here. Include conditional
; compile variables also.
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
; LOCAL FUNCTION DEFINITIONS
; Function Prototype declaration
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
; LOCAL STORE/BUFFER/POINTER DEFINITIONS
; Variable declaration - defined here and used outside this module
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
; EXTERNAL FUNCTION REFERENCES
; Declare functions defined elsewhere and referenced in this module
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
; EXTERNAL GLOBAL STORE/BUFFER/POINTER REFERENCES
; Declare variables used in this module but defined elsewhere
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
; FUNCTION CODE
----------------------------------------------------------------------------*/

void pns_left(
    const FrameInfo *pFrameInfo,
    const Int        group[],
    const Int        codebook_map[],
    const Int        factors[],
    const Int        sfb_prediction_used[],
    const Bool       ltp_data_present,
    Int32      spectral_coef[],
    Int        q_format[],
    Int32     *pCurrentSeed)
{

    Int     tot_sfb;
    Int     start_indx;

    Int     sfb;
    Int     band_stop;

    const Int16  *pBand;

    const Int *pCodebookMap = &(codebook_map[0]);
    const Int *pGroup   = &(group[0]);
    const Int *pFactors = &(factors[0]);

    Int     tempInt;
    Int32  *pWindow_Coef;


    Int32   *spec;

    Int partition;
    Int win_indx;

    tot_sfb = 0;

    spec = spectral_coef;

    /* PNS goes by group */
    win_indx = 0;
    partition = 0;
    do
    {
        Int num_bands = pFrameInfo->sfb_per_win[partition];
        pBand = pFrameInfo->win_sfb_top[partition];

        /*----------------------------------------------------------
        Partition is equal to the first window in the next group

        { Group 0    }{      Group 1      }{    Group 2 }{Group 3}
        [win 0][win 1][win 2][win 3][win 4][win 5][win 6][win 7]

        pGroup[0] = 2
        pGroup[1] = 5
        pGroup[2] = 7
        pGroup[3] = 8
        -----------------------------------------------------------*/

        partition = *pGroup++;      /* partition = index of last sbk in group */

        do
        {
            Int band_start = 0;
            for (sfb = 0; sfb < num_bands; sfb++)
            {
                band_stop = pBand[sfb]; /* band is offset table, band_stop is last coef in band */

                Int band_length =  band_stop - band_start;
                if (pCodebookMap[sfb] == NOISE_HCB)
                {

                    tempInt = sfb_prediction_used[tot_sfb] & ltp_data_present;

                    if (tempInt == FALSE)
                    {
                        /* generate random noise */
                        pWindow_Coef = spec + band_start;

                        tempInt = pFactors[sfb];

                        start_indx = tot_sfb++;

                        /* reconstruct noise substituted values */
                        /* generate random noise */

                        q_format[start_indx] = gen_rand_vector(pWindow_Coef,
                                                               band_length,
                                                               pCurrentSeed,
                                                               tempInt);

                    }   /* if (sfb_prediction_used[tot_sfb] == FALSE) */

                }   /* if (*(pCodebookMap++) == NOISE_HCB) */
                else
                {
                    tot_sfb ++;     /*  update absolute sfb counter  */
                }

                band_start = band_stop;

            }   /* for (sfb) */

            spec += pFrameInfo->coef_per_win[win_indx++];
            pFactors += num_bands;

        }
        while (win_indx < partition);

        pCodebookMap += pFrameInfo->sfb_per_win[win_indx-1];

    }
    while (partition < pFrameInfo->num_win);


    return;

} /* pns */
