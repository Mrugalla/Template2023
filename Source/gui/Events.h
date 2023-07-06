#pragma once
#include <vector>
#include <functional>

namespace gui
{
    namespace evt
    {
        enum class Type
        {
            TooltipUpdated,
            NumTypes
        };

		/* type, stuff */
        using Notify = std::function<void(const Type, const void*)>;

        struct System
        {
            struct Evt
            {
                Evt(System&);

                Evt(System&, const Notify&);

                Evt(System&, Notify&&);

                Evt(const Evt&);

                ~Evt();

                void operator()(const Type, const void* = nullptr) const;

                Notify notifier;
            protected:
                System& sys;
            };

            System();

			/* type, stuff */
            void notify(const Type, const void* = nullptr);

        protected:
            std::vector<Evt*> evts;

            void add(Evt*);

            void remove(const Evt*);
        };
    }
}