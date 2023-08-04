#ifndef UI_INFORMATIONDIALOG_H
#define UI_INFORMATIONDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_InfoDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *pathLabel;
    QLabel *sizeLabel;
    QLabel *createdLabel;
    QLabel *modifiedLabel;
    QLabel *permissionsLabel;
    QLabel *typeLabel;
    QPushButton *openButton;
    QSpacerItem *verticalSpacer;

    void setupUi(QDialog *InfoDialog)
    {
        if (InfoDialog->objectName().isEmpty())
            InfoDialog->setObjectName(QStringLiteral("InformationDialog"));
        InfoDialog->resize(400, 300);
        verticalLayout = new QVBoxLayout(InfoDialog);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        pathLabel = new QLabel(InfoDialog);
        pathLabel->setObjectName(QStringLiteral("pathLabel"));

        verticalLayout->addWidget(pathLabel);

        sizeLabel = new QLabel(InfoDialog);
        sizeLabel->setObjectName(QStringLiteral("sizeLabel"));

        verticalLayout->addWidget(sizeLabel);

        createdLabel = new QLabel(InfoDialog);
        createdLabel->setObjectName(QStringLiteral("createdLabel"));

        verticalLayout->addWidget(createdLabel);

        modifiedLabel = new QLabel(InfoDialog);
        modifiedLabel->setObjectName(QStringLiteral("modifiedLabel"));

        verticalLayout->addWidget(modifiedLabel);

        permissionsLabel = new QLabel(InfoDialog);
        permissionsLabel->setObjectName(QStringLiteral("permissionsLabel"));

        verticalLayout->addWidget(permissionsLabel);

        typeLabel = new QLabel(InfoDialog);
        typeLabel->setObjectName(QStringLiteral("typeLabel"));

        verticalLayout->addWidget(typeLabel);

        openButton = new QPushButton(InfoDialog);
        openButton->setObjectName(QStringLiteral("openButton"));

        verticalLayout->addWidget(openButton);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        retranslateUi(InfoDialog);

        QMetaObject::connectSlotsByName(InfoDialog);
    } // setupUi

    void retranslateUi(QDialog *InfoDialog)
    {
        InfoDialog->setWindowTitle(QCoreApplication::translate("InformationDialog", "Dialog", nullptr));
        openButton->setText(QCoreApplication::translate("InformationDialog", "Open File", nullptr));
    } // retranslateUi

};

namespace Ui {
    class InfoDialog: public Ui_InfoDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_INFORMATIONDIALOG_H
