#include <QScrollArea>

class PlainScrollArea : public QScrollArea
{
  Q_OBJECT
public:
  explicit PlainScrollArea(QWidget* parent = nullptr);
  
  virtual QSize sizeHint() const override;
};
