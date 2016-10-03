#ifndef __GRANDET_SDK_CPP_STATUSOR_H_
#define __GRANDET_SDK_CPP_STATUSOR_H_

enum ErrorCode {
    ERROR_OK,
    ERROR_FAIL
};

template<class T>
class StatusOr {
public:
    bool ok() const {
        return error == ERROR_OK;
    }

    ErrorCode error;
    T value;

    StatusOr(T& value) {
        this->value.swap(value);
        error = ERROR_OK;
    }

    StatusOr(ErrorCode error) {
        this->error = error;
    }
};

#endif // __GRANDET_SDK_CPP_STATUSOR_H_
