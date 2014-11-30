// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// solve.h
//
// THIS SOFTWARE IS PROVIDED BY CLINT WEISBROD "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL CLINT WEISBROD OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// ----------------------------------------------------------------------------

#pragma once

#include <utility>


// Solve a function using the bisection method.  Returns a pair
// with the solution as the first element and the error as the second.
template<class T, class F> std::pair<T, T> solve_bisection(F f,
                                                           T lower, T upper,
                                                           T err,
                                                           int maxIter = 100)
{
    T x = 0.0;

    for (int i = 0; i < maxIter; i++)
    {
        x = (lower + upper) * (T) 0.5;
        if (upper - lower < 2 * err)
            break;

        T y = f(x);
        if (y < 0)
            lower = x;
        else
            upper = x;
    }

    return std::make_pair(x, (upper - lower) / 2);
}


// Solve using iteration; terminate when error is below err or the maximum
// number of iterations is reached.
template<class T, class F> std::pair<T, T> solve_iteration(F f,
                                                           T x0,
                                                           T err,
                                                           int maxIter = 100)
{
    T x = 0;
    T x2 = x0;

    for (int i = 0; i < maxIter; i++)
    {
        x = x2;
        x2 = f(x);
        if (abs(x2 - x) < err)
            return std::make_pair(x2, x2 - x);
    }

    return std::make_pair(x2, x2 - x);
}


// Solve using iteration method and a fixed number of steps.
template<class T, class F> std::pair<T, T> solve_iteration_fixed(F f,
                                                                 T x0,
                                                                 int maxIter)
{
    T x = 0;
    T x2 = x0;

    for (int i = 0; i < maxIter; i++)
    {
        x = x2;
        x2 = f(x);
    }

    return std::make_pair(x2, x2 - x);
}
