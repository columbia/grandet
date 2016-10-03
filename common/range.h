#ifndef _GRANDET_RANGE_H_
#define _GRANDET_RANGE_H_

#include <experimental/optional>

struct Range {
    std::experimental::optional<int64_t> start;
    std::experimental::optional<int64_t> end;

    constexpr explicit operator bool() const {
        return start || end;
    }
};

#endif // _GRANDET_RANGE_H_
