#pragma once

#include"system_error.h"

typedef struct {
    char* ptr;
    cvv_errc ec;
} cvv_to_chars_result;

typedef struct {
    const char* ptr;
    cvv_errc ec;
} cvv_from_chars_result;

typedef enum {
    cvv_chars_format_scientific = 1,
    cvv_chars_format_fixed = 2,
    cvv_chars_format_hex = 4,
    cvv_chars_format_general = cvv_chars_format_fixed | cvv_chars_format_scientific
} cvv_chars_format;

// integer to chars

//template <class _RawTy>
_NODISCARD to_chars_result _Integer_to_chars(
    char* _First, char* const _Last, const _RawTy _Raw_value, const int _Base) noexcept {
    _Adl_verify_range(_First, _Last);
    _STL_ASSERT(_Base >= 2 && _Base <= 36, "invalid base in to_chars()");

    using _Unsigned = make_unsigned_t<_RawTy>;

    _Unsigned _Value = static_cast<_Unsigned>(_Raw_value);

    if constexpr (is_signed_v<_RawTy>) {
        if (_Raw_value < 0) {
            if (_First == _Last) {
                return { _Last, errc::value_too_large };
            }

            *_First++ = '-';

            _Value = static_cast<_Unsigned>(0 - _Value);
        }
    }

    constexpr size_t _Buff_size = sizeof(_Unsigned) * CHAR_BIT; // enough for base 2
    char _Buff[_Buff_size];
    char* const _Buff_end = _Buff + _Buff_size;
    char* _RNext = _Buff_end;

    switch (_Base) {
    case 10:
    { // Derived from _UIntegral_to_buff()
        // Performance note: Ryu's digit table should be faster here.
        constexpr bool _Use_chunks = sizeof(_Unsigned) > sizeof(size_t);

        if constexpr (_Use_chunks) { // For 64-bit numbers on 32-bit platforms, work in chunks to avoid 64-bit
                                     // divisions.
            while (_Value > 0xFFFF'FFFFU) {
                // Performance note: Ryu's division workaround would be faster here.
                unsigned long _Chunk = static_cast<unsigned long>(_Value % 1'000'000'000);
                _Value = static_cast<_Unsigned>(_Value / 1'000'000'000);

                for (int _Idx = 0; _Idx != 9; ++_Idx) {
                    *--_RNext = static_cast<char>('0' + _Chunk % 10);
                    _Chunk /= 10;
                }
            }
        }

        using _Truncated = conditional_t<_Use_chunks, unsigned long, _Unsigned>;

        _Truncated _Trunc = static_cast<_Truncated>(_Value);

        do {
            *--_RNext = static_cast<char>('0' + _Trunc % 10);
            _Trunc /= 10;
        } while (_Trunc != 0);
        break;
    }

    case 2:
        do {
            *--_RNext = static_cast<char>('0' + (_Value & 0b1));
            _Value >>= 1;
        } while (_Value != 0);
        break;

    case 4:
        do {
            *--_RNext = static_cast<char>('0' + (_Value & 0b11));
            _Value >>= 2;
        } while (_Value != 0);
        break;

    case 8:
        do {
            *--_RNext = static_cast<char>('0' + (_Value & 0b111));
            _Value >>= 3;
        } while (_Value != 0);
        break;

    case 16:
        do {
            *--_RNext = _Charconv_digits[_Value & 0b1111];
            _Value >>= 4;
        } while (_Value != 0);
        break;

    case 32:
        do {
            *--_RNext = _Charconv_digits[_Value & 0b11111];
            _Value >>= 5;
        } while (_Value != 0);
        break;

    case 3:
    case 5:
    case 6:
    case 7:
    case 9:
        do {
            *--_RNext = static_cast<char>('0' + _Value % _Base);
            _Value = static_cast<_Unsigned>(_Value / _Base);
        } while (_Value != 0);
        break;

    default:
        do {
            *--_RNext = _Charconv_digits[_Value % _Base];
            _Value = static_cast<_Unsigned>(_Value / _Base);
        } while (_Value != 0);
        break;
    }

    const ptrdiff_t _Digits_written = _Buff_end - _RNext;

    if (_Last - _First < _Digits_written) {
        return { _Last, errc::value_too_large };
    }

    _CSTD memcpy(_First, _RNext, static_cast<size_t>(_Digits_written));

    return { _First + _Digits_written, errc{} };
}


cvv_to_chars_result cvv_to_chars_c(char* first, char* last, char value, int base);
cvv_to_chars_result cvv_to_chars_sc(char* first, char* last, signed char value, int base);
cvv_to_chars_result cvv_to_chars_uc(char* first, char* last, unsigned char value, int base);
cvv_to_chars_result cvv_to_chars_s(char* first, char* last, short value, int base);
cvv_to_chars_result cvv_to_chars_us(char* first, char* last, unsigned short value, int base);
cvv_to_chars_result cvv_to_chars_i(char* first, char* last, int value, int base);
cvv_to_chars_result cvv_to_chars_ui(char* first, char* last, unsigned int value, int base);
cvv_to_chars_result cvv_to_chars_l(char* first, char* last, long value, int base);
cvv_to_chars_result cvv_to_chars_ul(char* first, char* last, unsigned long value, int base);
cvv_to_chars_result cvv_to_chars_ll(char* first, char* last, long long value, int base);
cvv_to_chars_result cvv_to_chars_ull(char* first, char* last, unsigned long long value, int base);
cvv_to_chars_result cvv_to_chars_b(char* first, char* last, _Bool value, int base);

// floating-point to chars

cvv_to_chars_result cvv_to_chars_f(char* first, char* last, float value);
cvv_to_chars_result cvv_to_chars_d(char* first, char* last, double value);
cvv_to_chars_result cvv_to_chars_ld(char* first, char* last, long double value);
cvv_to_chars_result cvv_to_chars_f_cf(char* first, char* last, float value, cvv_chars_format fmt);
cvv_to_chars_result cvv_to_chars_d_cf(char* first, char* last, double value, cvv_chars_format fmt);
cvv_to_chars_result cvv_to_chars_ld_cf(char* first, char* last, long double value, cvv_chars_format fmt);
cvv_to_chars_result cvv_to_chars_f_cf_i(char* first, char* last, float value, cvv_chars_format fmt, int precision);
cvv_to_chars_result cvv_to_chars_d_cf_i(char* first, char* last, double value, cvv_chars_format fmt, int precision);
cvv_to_chars_result cvv_to_chars_ld_cf_i(char* first, char* last, long double value, cvv_chars_format fmt, int precision);

// char to an integer value

cvv_from_chars_result cvv_from_chars_c(const char* first, const char* last, char* value, int base);
cvv_from_chars_result cvv_from_chars_sc(const char* first, const char* last, signed char* value, int base);
cvv_from_chars_result cvv_from_chars_uc(const char* first, const char* last, unsigned char* value, int base);
cvv_from_chars_result cvv_from_chars_s(const char* first, const char* last, short* value, int base);
cvv_from_chars_result cvv_from_chars_us(const char* first, const char* last, unsigned short* value, int base);
cvv_from_chars_result cvv_from_chars_i(const char* first, const char* last, int* value, int base);
cvv_from_chars_result cvv_from_chars_ui(const char* first, const char* last, unsigned int* value, int base);
cvv_from_chars_result cvv_from_chars_l(const char* first, const char* last, long* value, int base);
cvv_from_chars_result cvv_from_chars_ul(const char* first, const char* last, unsigned long* value, int base);
cvv_from_chars_result cvv_from_chars_ll(const char* first, const char* last, long long* value, int base);
cvv_from_chars_result cvv_from_chars_ull(const char* first, const char* last, unsigned long long* value, int base);

// char to a floating-point value

// base = 0
// fmt = chars_format::general

cvv_from_chars_result cvv_from_chars_f(const char* first, const char* last, float* value, cvv_chars_format fmt);
cvv_from_chars_result cvv_from_chars_d(const char* first, const char* last, double* value, cvv_chars_format fmt);
cvv_from_chars_result cvv_from_chars_ld(const char* first, const char* last, long double* value, cvv_chars_format fmt);