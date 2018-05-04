#ifndef GECKO_STROMER_STATUS_H
#define GECKO_STROMER_STATUS_H

typedef enum status_e
{
    status_failed       = 0,
    status_ok           = 1,
    status_errinternal,
    status_notimplemented,
    status_nulldetected,
    status_nosuchelement,
    status_continue,
    status_closed,
    status_illegalarg,
    status_reallocerr,
    status_duplicate,
    status_ambiguous,
    status_encodingerr,
    status_notfound,
    status_errassertion,
    status_typemissmatch
} status_e;

#endif //GECKO_STROMER_STATUS_H
