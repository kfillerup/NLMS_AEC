#include <stdint.h>
#include <stdlib.h>
/* NLMS Acousitc Echo Cancelation
 * Copyright (c) 2016, Sergey Makitrin, Kyler Fillerup
 *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


 /* NLMS algorithm takes several inputs. "Mic" array is an array containing signal
 from microphone. "x" signal is an array which contains samples from far end of
 the transmission. Both, 'Mic' and 'x' have to be same length. 'error' is an output
 array to near side speaker. error is calculated by comparing incoming far end
 signal 'x' and 'Mic' signal. If 'Mic' signal contains similarities to 'x' they will
 be eliminated. 'w' is the filter array. Typical length of 'w' is about 1024, which
 is about 8 blocks (128 samples/block) of audio signal. Length of the filter has to
 be one block less than 'Mic' or 'x' array. 'gg' is the index of l-1 block in 'Mic'
 and 'x'. For example if the length of 'Mic' is 1152 then gg will be 1152-128=1024.
 Finally, 'mu' is the step size parameter for conversion of NLMS algorithm. It is an
 integer, in this case. The actual value of 'mu' is mu/128. Higher 'mu' will result in
 faster convergence of the signal, but it will fail to eliminate all of the similarities
 between 'Mic' and 'x'. Smaller 'mu'  has slower convergence, but will eliminate
 similarities much better.

 For additional theory behind NLMS algorithm read this link:
 https://en.wikipedia.org/wiki/Least_mean_squares_filter

  */

void NLMS_AEC(int16_t *Mic, int16_t *x, int16_t *error, int32_t *w, int16_t gg, int64_t mu, int8_t n, int8_t jj){
  int64_t yhat = 0;
  static int64_t xtdl = 1;
  int64_t mu0 = 0;
  int16_t y12 = gg/2;
  int16_t out;
//  bool update = false;

  for(int h = 0; h<128;h+=n){
//        if(h% (10*n)==0)
//        {
//            xtdl = 0;
//            update = true;
//        }
    for(int j = 0; j<gg;j+=n){
      yhat += (x[gg-j+h]*w[j]);
//      if(update){
      xtdl += x[gg-j+h]*x[gg-j+h];
//    }
    }

    out = Mic[gg+h]-yhat/524288;
    for(int i = 0;i<n;i++){
        error[h+i] = out;
    }

    yhat = 0;
    mu0 = (mu*out)/(xtdl);
    xtdl = 1;
    //update filter taps
    if(jj == 1){
    for(int j = 0; j<y12;j+=n){
        w[j] = w[j] + (x[gg-j+h]*mu0)/2097152;
        w[y12+j] = w[y12+j] + (x[y12-j+h]*mu0)/2097152;
    }//end for
    }//end if
  }//end for outer
//  xtdl = 0;
  //return xtdl;
}// end NLMS_AEC
