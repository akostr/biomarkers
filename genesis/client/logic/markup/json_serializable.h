#ifndef JSONSERIALIZABLE_H
#define JSONSERIALIZABLE_H

#include <QJsonObject>

class JsonSerializable
{
public:
  JsonSerializable(){};
  virtual ~JsonSerializable(){};
  virtual void load(const QJsonObject& data) = 0;
  virtual QJsonObject save() const = 0;
};

#endif // JSONSERIALIZABLE_H
