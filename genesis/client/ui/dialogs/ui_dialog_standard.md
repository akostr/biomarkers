# Стандартный подход к созданию диалога на WebDialog

#### Структура WebDialog



##### *Конструктор*

```c++
WebDialog(QWidget* parent, QWidget* content = nullptr, QSizeF size = QSizeF(),
            QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok);
```

в качестве `parent` я просто всегда передаю текущий виджет, из которого вызываю диалог, и не парюсь.

`content` - содержимое диалога, можно передать из вне, тогда он установится как основной контент, и доступ к нему изнутри диалога есть через мэмбер диалога: `WebDialog::Content`. Если `content` равен `nullptr`, то в конструкторе диалога создается новый виджет, его указатель сохраняется в мэмбер `WebDialog::Content`.

`size` - значение в диапазоне от `(0,0)` до `(1, 1)`, позволяет задать предпочитаемый размер виджета в % от ширины и высоты основного окна. Если виджет не помещается в обозначенные размеры, он расширится, но будет стараться быть как можно компактнее. Если же виджет слишком маленький, он наоборот, растянется.

`buttons` - набор кнопок внизу диалога. Например: `QDialogButtonBox::Yes | QDialogButtonBox::YesToAll | QDialogButtonBox::Cancel` добавит три соответствующие кнопки. 
К каждой из этих кнопок можно обратиться как к `QPushButton*` через мэмбер `WebDialog::ButtonBox`, подписаться на ее сигналы, изменить текст и прочее: 

```c++
QPushButton* btn = ButtonBox->button(QDialogButtonBox::YesToAll);
```



##### *Члены класса*

```c++
  QPointer<QWidget> Overlay;
  QSizeF            Size;
  int               Result;

  QVBoxLayout*      MainLayout;
  QWidget*          Content;
  QHBoxLayout*      ButtonLayout;
  QDialogButtonBox* ButtonBox;
```

`Overlay` - мы вообще не трогаем.

`Size` - размеры окна диалога в %

`Result` - результат. Если в диалоге вызвать `void WebDialog::Done(int r)`, то при r == QDialog::Accepted или r == QDialog::Rejected будет вызвано соответствующее событие: `Accepted()` или `Rejected()`. А так же, в любом случае будет вызвано `Finished(r)`. 

`MainLayout` - главный лэйаут, в котором находятся `Content` и `ButtonLayout`.

`Content` - содержимое диалога (QWidget*). Либо передается извне, либо автоматически создается в конструкторе.

`ButtonLayout`  и `ButtonBox` - элементы внизу диалога. Кнопки, и их лэйаут.



##### *Методы класса*



###### публичные

```c++
public:
  void SetButtonName(QDialogButtonBox::StandardButton button, const QString& name);
  int GetResult();
  static WebDialog* Question(QString question, QWidget *parent);
```

`SetButtonName` - установить название кнопки из `ButtonBox`

`GetResult` - возвращает текущий `Result`

`Question` - статическая функция, которая возвращает стандартный диалог подтверждения. Все равно требуется вызвать `dial->Open();`



###### слоты

```c++
public slots:
  virtual void Open();
  virtual void Accept();
  virtual void Done(int r);
  virtual void Reject();
```

`Open` - без вызова этой функции ничего не будет работать. Отрывает и отображает сам диалог.

`Accept` - виртуальная функция, которая вызывается при нажатии на кнопку из `ButtonBox` с ролью `QDialog::Accepted`. Если нужно ввести какие то проверки содержимого, или переключения на другие страницы, просто переопределите эту функцию. По умолчанию в ней вызывается `Done(QDialog::Accepted);`

`Reject` - полностью аналогична `Accept` но с ролью `QDialog::Rejected`

`Done` - завершает работу диалога, записывая в `Result` значение из `r`



###### сигналы

```c++
signals:
  void Accepted();
  void Finished(int result);
  void Rejected();
  void Clicked(QDialogButtonBox::StandardButton button);
```

`Accepted` - сигнал, посылаемый при завершении работы диалога, если была нажата кнопка с ролью `QDialog::Accepted`.

`Rejected` - аналогично, с ролью `QDialog::Rejected`.

`Clicked` - посылается, если кто-то нажал какую либо кнопку в `ButtonBox`

`Finished` - посылается при завершении работы диалога в любом случае.



###### protected

```c++
protected:
  void UpdateGeometry();
```

`UpdateGeometry` - функция, отвечающая за геометрию диалога. 



#### Стандартный подход:

Мы решили, что будем все диалоги делать через UI - формы, что бы не нужно было потом другим людям пол часа изучать строчки кода, и думать, кто же в кого вложен и какая ж там иерархия виджетов.

Так что создаем UI форму.
	варианты:  

- создать отдельно UI форму и подключить к своей имплементации `WebDialog`;

   - создать сразу класс с UI формой, и отнаследовать его от `WebDialog`

начнем с первого:

###### Создание отдельной UI формы

не важно в каком порядке, но нужно 

а) создать класс

б) создать форму.

Начнем с пункта а):

**Ремарка:** Диалогов у нас уже даже слишком много, так что лучше все таки уже создавать поддиректории по темам.

Создаем свой класс, или берем уже существующий. Например:
```c++
#include "web_dialog.h"

class DialogExample : public WebDialog
{
  Q_OBJECT
public:
  DialogExample(QWidget* parent, QSizeF size = QSizeF(),
            QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok);
};
```

Обратите внимание, что из конструктора исключена передача `QWidget* content`. Это не обязательно делать именно так, но я сделал так, и в самом конце будет понятно, почему именно так.



прописываем в CMake наши файлы в соответствующие места.

Теперь создаем UI форму (Qt Designer Form, а не Qt Designer Form Class) на основе Widget в той же папке, в которой мы создали класс наследник от `WebDialog`.

прописываем в CMake наш файл в соответствующее место.

Допустим, получившиеся файлы:

- `dialog_example.h`;
- `dialog_example.cpp`;
- `dialog_example.ui`.

**ВАЖНО! ** Теперь в форме нужно дать правильное название главному виджету через дизайнер. Открываем форму в дизайнере, и называем главный виджет `DialogExample`.  Это обязательно нужно сделать, потому что именно так будет называться класс нашего виджета после автогенерации файлов.

Теперь нужно запустить сборку, что бы моки всякие отработали, и создались нужные файлы.

Переходим в `dialog_example.cpp`:

```c++
#include "dialog_example.h"
//добавляем сюда инклюд: 
#include "ui_dialog_example.h"

DialogExample::DialogExample(QWidget *parent, QSizeF size, QDialogButtonBox::StandardButtons buttons)
  : WebDialog(parent, nullptr, size, buttons)
{

}
```

Обратите внимание, что из конструктора исключена передача `QWidget* content`, а в конструктор `WebDialog`  передается вместо него `nullptr`. Это сделано для того, что бы WebDialog сам создал пустой виджет, и записал его указатель в `WebDialog::Content`. Я так сделал, потому что хочу что бы вообще весь виджет Content был целиком создан в дизайнере. Но это не обязательно, подключаемый нами UI можно установить в любой виджет.



добавляем туда `#include "ui_dialog_example.h"`. название файла - это название UI файла: `dialog_example.ui` + префикс `ui_`, но с расширением `.h`.

можно попробовать сбилдить опять, на всякий, что бы проверить, что название указано правильно.

Залезаем в `ui_dialog_example.h` и видим:
```c++
/********************************************************************************
** Form generated from reading UI file 'dialog_example.ui'
**
** Created by: Qt User Interface Compiler version 6.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOG_EXAMPLE_H
#define UI_DIALOG_EXAMPLE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DialogExample
{
public:

    void setupUi(QWidget *DialogExample)
    {
        if (DialogExample->objectName().isEmpty())
            DialogExample->setObjectName(QString::fromUtf8("DialogExample"));
        DialogExample->resize(400, 300);

        retranslateUi(DialogExample);

        QMetaObject::connectSlotsByName(DialogExample);
    } // setupUi

    void retranslateUi(QWidget *DialogExample)
    {
        DialogExample->setWindowTitle(QCoreApplication::translate("DialogExample", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DialogExample: public Ui_DialogExample {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOG_EXAMPLE_H
```



в самом низу то что нам нужно:

```c++
namespace Ui {
    class DialogExample: public Ui_DialogExample {};
} // namespace Ui
```

Обратите внимание, что название класса `DialogExample` - это то название, которое мы давали главному виджету в дизайнере форм.

копируем в `dialog_example.h`, и чуть чуть модифицируем, а так же добавляем переменную `mUi`:

```c++
#ifndef DIALOGEXAMPLE_H
#define DIALOGEXAMPLE_H

#include "web_dialog.h"
////////////НОВОЕ//////////////////  
namespace Ui {                   //
    class DialogExample;         //
} // namespace Ui                //
///////////////////////////////////

class DialogExample : public WebDialog
{
  Q_OBJECT
public:
  DialogExample(QWidget* parent, QSizeF size = QSizeF(),
                QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok);
////////////НОВОЕ//////////////////  
private:                         //
  Ui::DialogExample* mUi;        //
///////////////////////////////////    
};

#endif // DIALOGEXAMPLE_H
```

теперь телепортируемся в `dialog_example.cpp`, и дополняем конструктор инициализацией `mUi` и установкой UI в `WebDialog::Content`:
```c++
#include "dialog_example.h"
#include "ui_dialog_example.h"

DialogExample::DialogExample(QWidget *parent, QSizeF size, QDialogButtonBox::StandardButtons buttons)
  : WebDialog(parent, nullptr, size, buttons),
    mUi(new Ui::DialogExample)
{
  mUi->setupUi(Content);
}
```

Обратите внимание, что из конструктора исключена передача `QWidget* content`, а в конструктор `WebDialog`  передается вместо него `nullptr`. Это сделано для того, что бы WebDialog сам создал пустой виджет, и записал его указатель в `WebDialog::Content`. Я так сделал, потому что хочу что бы вообще весь виджет Content был целиком создан в дизайнере. Но это не обязательно, подключаемый нами UI можно установить в любой виджет:

достаточно в `mUi->setupUi` передать любой виджет, в котором мы хотим установить наш UI. Я же напрямую передаю туда `WebDialog::Content`, потому что знаю точно, что этот виджет будет пустым, т.к. его создаст конструктор `WebDialog`, потому что я передал туда `nullptr`.

Вот и все, теперь UI, созданный в дизайнере, будет установлен в виджет Content!



###### Создание сразу Ui Designer Form Class:

1. Правой кликой по папочке в бровзере проектов -> add new -> Qt -> Qt Designer Form Class.

2. Задаем название класса, задаем названия файлов:

   - DialogFormClassExample

   - dialog_form_class_example.h

   - dialog_form_class_example.cpp

   - dialog_form_class_example.ui

3. Ok

Прописываем все файлы в CMake.

идем в `dialog_form_class_example.h`:

```c++
#ifndef DIALOG_FORM_CLASS_EXAMPLE_H
#define DIALOG_FORM_CLASS_EXAMPLE_H

#include <QWidget>

namespace Ui {
class DialogFormClassExample;
}

class DialogFormClassExample : public QWidget
{
  Q_OBJECT

public:
  explicit DialogFormClassExample(QWidget *parent = nullptr);
  ~DialogFormClassExample();

private:
  Ui::DialogFormClassExample *ui;
};

#endif // DIALOG_FORM_CLASS_EXAMPLE_H
```

 меняем в соответствии с `WebDialog`:

```c++
#ifndef DIALOG_FORM_CLASS_EXAMPLE_H
#define DIALOG_FORM_CLASS_EXAMPLE_H

#include "web_dialog.h"  //поменяли

namespace Ui {
class DialogFormClassExample;
}

class DialogFormClassExample : public WebDialog
{
  Q_OBJECT

public:
  //поменяли:
  explicit DialogFormClassExample(QWidget* parent, QSizeF size = QSizeF(),
                                  QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok);
  //
  ~DialogFormClassExample();

private:
  Ui::DialogFormClassExample *ui;
};

#endif // DIALOG_FORM_CLASS_EXAMPLE_H
```

идем в `dialog_form_class_example.cpp`:

```c++
#include "ui_dialog_form_class_example.h"

DialogFormClassExample::DialogFormClassExample(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::DialogFormClassExample)
{
  ui->setupUi(this);
}

DialogFormClassExample::~DialogFormClassExample()
{
  delete ui;
}
```

приводим его к виду:

```c++
#include "dialog_form_class_example.h"
#include "ui_dialog_form_class_example.h"
//поменяли:
DialogFormClassExample::DialogFormClassExample(QWidget *parent, QSizeF size, QDialogButtonBox::StandardButtons buttons) :
  WebDialog(parent, nullptr, size, buttons),
//
  ui(new Ui::DialogFormClassExample)
{
  ui->setupUi(Content);
}

DialogFormClassExample::~DialogFormClassExample()
{
  delete ui;
}
```



# Готово!