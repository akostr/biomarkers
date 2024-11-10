#pragma once

#include <QSharedPointer>
#include <QWeakPointer>

#define DefineClassS(type) class type; typedef QSharedPointer<type> type##S; typedef QWeakPointer<type> type##W
#define DefineStructS(type) struct type; typedef QSharedPointer<type> type##S; typedef QWeakPointer<type> type##W
