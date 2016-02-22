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

int16_t *NLMS_AEC(int16_t *Mic, int16_t *x, int16_t *error, int16_t *w, int16_t gg, int16_t mu){
  int16_t yhat = 0;
  int64_t xtdl = 0;
  const int8_t psi = 1;
  int16_t mu0 = 0;
  for(int h = 0; h<128;h++){
    for(int j = gg; j >0;j--){
      yhat += (x[j+h]*w[gg-j])/32768;
      xtdl += x[j+h]*x[j+h];
    }
    error[h] = Mic[gg+h]-yhat;
    yhat = 0;
    xtdl = xtdl + psi;
    mu0 = (67108864*mu)/xtdl;
    xtdl = 0;

    //update filter taps
    for(int j = 0; j<gg;j++){
      w[j] = w[j] + (x[gg-j+h]*mu0*error[h])/131072;
    }//end for
  }//end for outer
  return error;
}// end NLMS_AEC
