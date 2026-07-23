#include <functional>
#include <vector>

using SubscriptionId = uint8_t;

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
    void publish(const T& event)
    {
        for (const auto& subscription : subscriptions)
        {
            subscription(event);
        }
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
