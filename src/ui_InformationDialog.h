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

class Ui_InformationDialog
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

    void setupUi(QDialog *InformationDialog)
    {
        if (InformationDialog->objectName().isEmpty())
            InformationDialog->setObjectName(QStringLiteral("InformationDialog"));
        InformationDialog->resize(400, 300);
        verticalLayout = new QVBoxLayout(InformationDialog);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        pathLabel = new QLabel(InformationDialog);
        pathLabel->setObjectName(QStringLiteral("pathLabel"));

        verticalLayout->addWidget(pathLabel);

        sizeLabel = new QLabel(InformationDialog);
        sizeLabel->setObjectName(QStringLiteral("sizeLabel"));

        verticalLayout->addWidget(sizeLabel);

        createdLabel = new QLabel(InformationDialog);
        createdLabel->setObjectName(QStringLiteral("createdLabel"));

        verticalLayout->addWidget(createdLabel);

        modifiedLabel = new QLabel(InformationDialog);
        modifiedLabel->setObjectName(QStringLiteral("modifiedLabel"));

        verticalLayout->addWidget(modifiedLabel);

        permissionsLabel = new QLabel(InformationDialog);
        permissionsLabel->setObjectName(QStringLiteral("permissionsLabel"));

        verticalLayout->addWidget(permissionsLabel);

        typeLabel = new QLabel(InformationDialog);
        typeLabel->setObjectName(QStringLiteral("typeLabel"));

        verticalLayout->addWidget(typeLabel);

        openButton = new QPushButton(InformationDialog);
        openButton->setObjectName(QStringLiteral("openButton"));

        verticalLayout->addWidget(openButton);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        retranslateUi(InformationDialog);

        QMetaObject::connectSlotsByName(InformationDialog);
    } // setupUi

    void retranslateUi(QDialog *InformationDialog)
    {
        InformationDialog->setWindowTitle(QCoreApplication::translate("InformationDialog", "Dialog", nullptr));
        openButton->setText(QCoreApplication::translate("InformationDialog", "Open File", nullptr));
    } // retranslateUi

};

namespace Ui {
    class InformationDialog: public Ui_InformationDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_INFORMATIONDIALOG_H
