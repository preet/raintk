/*
This method adapted from PNPOLY (https://www.ecse.rpi.edu/Homepages/wrf/Research/Short_Notes/pnpoly.html)

Copyright (c) 1970-2003, Wm. Randolph Franklin

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimers.
    Redistributions in binary form must reproduce the above copyright notice in the documentation and/or other materials provided with the distribution.
    The name of W. Randolph Franklin may not be used to endorse or promote products derived from this Software without specific prior written permission.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

namespace
{
    // * Determines whether @test_point lies within @poly_vx
    bool CalcPointInPoly(std::vector<glm::vec2> const &poly_vx,
                         glm::vec2 const &test_point)
    {
        int i=0;
        int j=0;
        bool c = false;
        int nvert = poly_vx.size();

        for (i = 0, j = nvert-1; i < nvert; j = i++)
        {
          if ( ((poly_vx[i].y>test_point.y) != (poly_vx[j].y>test_point.y)) &&
                (test_point.x < (poly_vx[j].x-poly_vx[i].x) * (test_point.y-poly_vx[i].y) /
                (poly_vx[j].y-poly_vx[i].y) + poly_vx[i].x) )
          {
              c = !c;
          }
        }
        return c;
    }
}

