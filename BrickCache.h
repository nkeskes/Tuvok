#ifndef TUVOK_BRICK_CACHE_H
#define TUVOK_BRICK_CACHE_H

#include <memory>
#include <vector>
#include "Brick.h"

namespace tuvok {

// Implements a simple brick cache: associates a chunk of data with the given
// brick key.
// Lookup of a nonexistent key results in an empty vector: there is no way to
// tell whether a key doesn't exist, or whether the data it stores is actually
// empty.
class BrickCache {
  public:
    BrickCache();
    ~BrickCache();

    /// the second arguments here aren't used, they are just a type tag.
    /// so, for example:
    ///   std::vector<uint32_t> v = cache.lookup(key, uint32_t());
    /// i.e. just give it an empty value so it knows which one to call.
    ///@{
    std::vector< uint8_t> lookup(const BrickKey&, uint8_t);
    std::vector<uint16_t> lookup(const BrickKey&, uint16_t);
    std::vector<uint32_t> lookup(const BrickKey&, uint32_t);
    std::vector<uint64_t> lookup(const BrickKey&, uint64_t);
    std::vector< int8_t> lookup(const BrickKey&, int8_t);
    std::vector<int16_t> lookup(const BrickKey&, int16_t);
    std::vector<int32_t> lookup(const BrickKey&, int32_t);
    std::vector<int64_t> lookup(const BrickKey&, int64_t);
    std::vector<float> lookup(const BrickKey&, float);
    ///@}

    /// These return their argument for ease of use.
    ///@{
    std::vector< uint8_t>& add(const BrickKey&, std::vector<uint8_t>&);
    std::vector<uint16_t>& add(const BrickKey&, std::vector<uint16_t>&);
    std::vector<uint32_t>& add(const BrickKey&, std::vector<uint32_t>&);
    std::vector<uint64_t>& add(const BrickKey&, std::vector<uint64_t>&);
    std::vector< int8_t>& add(const BrickKey&, std::vector<int8_t>&);
    std::vector<int16_t>& add(const BrickKey&, std::vector<int16_t>&);
    std::vector<int32_t>& add(const BrickKey&, std::vector<int32_t>&);
    std::vector<int64_t>& add(const BrickKey&, std::vector<int64_t>&);
    std::vector<float>& add(const BrickKey&, std::vector<float>&);
    ///@}

    void remove(const BrickKey&);
  private:
    struct bcinfo;
    std::unique_ptr<bcinfo> ci;
};
}

#endif
/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2013 Scientific Computing and Imaging Institute,
   University of Utah.


   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/
