/*
  Copyright (C) 2015-2016 Preet Desai (preet.desai@gmail.com)

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/


#include <raintk/test/RainTkTestContext.hpp>
#include <raintk/RainTkDrawableWidget.hpp>
#include <raintk/RainTkRectangle.hpp>
#include <raintk/RainTkImage.hpp>

using namespace raintk;

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    auto a = DrawableWidget::GetTypeId<int>();
    auto b = DrawableWidget::GetTypeId<float>();
    auto c = DrawableWidget::GetTypeId<double>();
    auto d = DrawableWidget::GetTypeId<Rectangle>();
    auto e = DrawableWidget::GetTypeId<Image>();

    // All Type Ids should be different
    rtklog.Trace() << a;
    rtklog.Trace() << b;
    rtklog.Trace() << c;
    rtklog.Trace() << d;
    rtklog.Trace() << e;

    return 0;
}

