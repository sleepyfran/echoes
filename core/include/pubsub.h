#pragma once

#include <cstdint>
#include <functional>
#include <vector>

using SubscriptionId = uint8_t;

/**
 * A restricted capability that allows a collaborator to publish events without exposing the
 * underlying pub-sub.
 */
template <typename T> using Publisher = std::function<void(const T&)>;

/**
 * Encapsulates a progress reporter that can accept subscriptions and raises events to all of them
 * as they happen.
 */
template <typename T> class PubSub
{
    std::vector<std::function<void(T)>> subscriptions;

  protected:
    /**
     * Publishes a new event to all subscribers.
     */
    void publish(const T& event) const
    {
        for (const auto& subscription : subscriptions)
        {
            subscription(event);
        }
    }

    /**
     * Creates a publishing capability that can be passed to collaborators without exposing publish
     * to consumers of this pub-sub.
     */
    [[nodiscard]] Publisher<T> publisher() const
    {
        return [this](const T& event) { publish(event); };
    }

  public:
    /**
     * Subscribes to the events raised by the reporter, returning a subscription ID that can be used
     * to stop listening to events.
     */
    [[nodiscard]] SubscriptionId subscribe(std::function<void(T)> on_event)
    {
        subscriptions.push_back(std::move(on_event));
        return subscriptions.size() - 1;
    }

    /**
     * Unsubscribes from the given subscription ID. Non-existing subscriptions that match that ID is
     * a no-op.
     */
    void unsubscribe(SubscriptionId id)
    {
        const unsigned long current_count = subscriptions.size();
        if (id < current_count && id > current_count)
        {
            return;
        }

        subscriptions.erase(subscriptions.begin() + id);
    }
};
