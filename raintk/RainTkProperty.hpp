/*
   Copyright (C) 2015 Preet Desai (preet.desai@gmail.com)

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

#ifndef RAINTK_PROPERTY_HPP
#define RAINTK_PROPERTY_HPP

#include <raintk/RainTkGlobal.hpp>
#include <ks/shared/KsDynamicProperty.hpp>

namespace raintk
{
    // ============================================================= //

    template<typename T>
    class DynamicPropertyPushNotify : public ks::DynamicProperty<T>
    {
    public:
        // Need to explicitly have this to prevent compiler
        // generated assignment operators from overriding
        // and hiding the one we want to call
        using ks::DynamicProperty<T>::operator=;

        DynamicPropertyPushNotify(T value) :
            ks::DynamicProperty<T>(std::move(value))
        {
            this->setupNotifier();
        }

        DynamicPropertyPushNotify(
                typename ks::DynamicProperty<T>::BindingFn binding) :
            ks::DynamicProperty<T>(std::move(binding))
        {
            this->setupNotifier();
        }

        mutable ks::Signal<T> signal_changed;

    private:
        void setupNotifier()
        {
            this->SetNotifier(
                        [this](T const &v) {
                            signal_changed.Emit(v);
                        });
        }
    };

    // ============================================================= //

    template<typename T>
    class DynamicPropertyPullNotify : public ks::DynamicProperty<T>
    {
    public:
        using ks::DynamicProperty<T>::operator=;

        DynamicPropertyPullNotify(T value) :
            ks::DynamicProperty<T>(std::move(value))
        {
            this->setupNotifier();
        }

        DynamicPropertyPullNotify(
                typename ks::DynamicProperty<T>::BindingFn binding) :
            ks::DynamicProperty<T>(std::move(binding))
        {
            this->setupNotifier();
        }

        mutable ks::Signal<> signal_changed;

    private:
        void setupNotifier()
        {
            this->SetNotifier(
                        [this](T const &) {
                            signal_changed.Emit();
                        });
        }
    };

    template<typename T>
    using Property = DynamicPropertyPullNotify<T>;

    // ============================================================= //
}

#endif // RAINTK_PROPERTY_HPP
