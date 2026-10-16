#include "rate_control.hpp"
#include <algorithm>

namespace orbis {
RateControl::RateControl(bool buffered, int64_t lead)
    : window(buffered ? 10000 : 6000), maxDrift(buffered ? 6000 : 2000), backlog(lead / 2),
      probe(buffered ? 180000 : 120000), baseProbe(probe) {}

void RateControl::settle(int64_t now) {
    count = 0;
    settleUntil = now + 5000;
    cleanSince = now;
}

std::optional<int> RateControl::observe(DeliverySample sample, int nominal, int audio) {
    if (nominal <= 0)
        return {};
    if (cap && *cap >= nominal)
        cap.reset();
    if (settleUntil < 0) {
        lastChange = sample.ticks;
        settle(sample.ticks);
    }
    if (sample.ticks < settleUntil)
        return {};
    if (count) {
        const auto& last = samples[newest];
        if (sample.media < last.media || sample.bytes < last.bytes) {
            settle(sample.ticks);
            return {};
        }
        if (sample.ticks - last.ticks < 200)
            return {};
    } else {
        origin = sample;
        bestAhead = 0;
    }
    newest = (newest + 1) % samples.size();
    samples[newest] = sample;
    count = std::min(count + 1, samples.size());
    const auto ahead = sample.media - origin.media - (sample.ticks - origin.ticks);
    bestAhead = std::max(bestAhead, ahead);
    const auto drift = bestAhead - ahead;
    auto oldest = sample, recent = sample;
    int64_t lead = 0, readings = 0;
    for (size_t i = 0; i < count; ++i) {
        const auto& entry = samples[(newest + samples.size() - i) % samples.size()];
        const auto age = sample.ticks - entry.ticks;
        if (age > window)
            break;
        oldest = entry;
        if (age <= window / 3)
            recent = entry;
        lead += entry.lead;
        ++readings;
    }
    const auto span = sample.ticks - oldest.ticks, shortSpan = sample.ticks - recent.ticks;
    if (span < window * 4 / 5 || shortSpan <= 0)
        return {};
    const double rate = double(sample.media - oldest.media) / span;
    const double shortRate = double(sample.media - recent.media) / shortSpan;
    const double throughput = double(sample.bytes - recent.bytes) * 8 / shortSpan;
    const int current = cap.value_or(nominal);
    if (lead / readings >= backlog && shortRate >= .1 && shortRate <= 1 && drift >= maxDrift) {
        cleanSince = sample.ticks;
        const int floor = std::min(nominal, std::max(500, int(nominal * .3)));
        const int carried = int(throughput * .85) - audio;
        int target = std::clamp(carried, int(current * .5), int(current * .85));
        target = std::max(floor, target - target % 10);
        if (target >= current)
            return {};
        if (probing) {
            probe = sample.ticks - lastChange < probe ? std::min(probe * 2, int64_t(960000))
                                                      : baseProbe;
            probing = false;
        }
        cap = target;
        lastChange = sample.ticks;
        settle(sample.ticks);
        return target;
    }
    if (drift >= 500 || rate < .97)
        cleanSince = sample.ticks;
    if (cap && sample.ticks - cleanSince >= probe && sample.ticks - lastChange >= probe) {
        int target = current + int(nominal * .2);
        target = std::min(nominal, target - target % 10);
        if (target >= nominal)
            cap.reset();
        else
            cap = target;
        if (probing)
            probe = baseProbe;
        probing = true;
        lastChange = sample.ticks;
        settle(sample.ticks);
        return target;
    }
    return {};
}
} // namespace orbis
