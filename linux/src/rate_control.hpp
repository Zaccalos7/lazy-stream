#pragma once
#include "paced_output.hpp"
#include <array>
#include <optional>

namespace orbis {
// Same delivery windows and step bounds as Windows BitrateLadder, in kbit/s internally.
class RateControl {
    std::array<DeliverySample, 128> samples{};
    size_t count = 0, newest = 0;
    int64_t window, maxDrift, backlog, probe, baseProbe, settleUntil = -1;
    int64_t cleanSince = 0, lastChange = 0, bestAhead = 0;
    DeliverySample origin;
    std::optional<int> cap;
    bool probing = false;
    void settle(int64_t now);

  public:
    RateControl(bool buffered, int64_t maxLead);
    std::optional<int> observe(DeliverySample sample, int nominalKbps, int audioKbps);
};
} // namespace orbis
