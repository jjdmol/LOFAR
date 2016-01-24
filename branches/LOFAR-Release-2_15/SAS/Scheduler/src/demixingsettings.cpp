#include "demixingsettings.h"

QStringList DemixingSettings::demixAlwaysList(void) const {
    QStringList lst;
    if (!itsDemixAlways.isEmpty()) {
        lst = itsDemixAlways.split(',');
    }
    return lst;
}

QStringList DemixingSettings::demixIfNeededList(void) const {
    QStringList lst;
    if (!itsDemixIfNeeded.isEmpty()) {
        lst = itsDemixIfNeeded.split(',');
    }
    return lst;
}
