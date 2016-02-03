static const char digit_pairs[201] = {
    "00010203040506070809"
    "10111213141516171819"
    "20212223242526272829"
    "30313233343536373839"
    "40414243444546474849"
    "50515253545556575859"
    "60616263646566676869"
    "70717273747576777879"
    "80818283848586878889"
    "90919293949596979899"
};

static void to_string(char *buffer, uint32_t num)
{
    if (!num) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    int size;
    if (num >= 10000) {
        if (num >= 10000000)
            size = num >= 1000000000 ? 10 : 8 + (num >= 100000000);
        else
            size = num >= 1000000 ? 7 : 5 + (num >= 100000);
    } else
        size = num >= 100 ? 3 + (num >= 1000) : 1 + (num >= 10);

    buffer[size] = '\0';

    char *c = &buffer[size - 1];
    while (num >= 100) {
       int pos = num % 100;
       num /= 100;
       *(uint16_t *)(c - 1) = *(uint16_t *)(digit_pairs + 2 * pos);
       c -= 2;
    }
    while (num) {
        *c-- = '0' + (num % 10);
        num /= 10;
    }
}

static int is_small_unsigned(const char *string)
{
    char c;
    int numeric = 0;
    const char *ptr = string;
    while ((c = *ptr++)) {
        if (c < '0' || c > '9' || numeric > 10) {
            numeric = -1;
            break;
        }
        numeric++;
    }
    return numeric >= 1 && \
        (numeric < 10 || (numeric == 10 && string[0] <= '2'));
}

static uint32_t to_unsigned(const char *string)
{
    uint32_t number = 0;
    char c;
    while ((c = *string++))
        number = number * 10 + (c - '0');
    return number;
}
