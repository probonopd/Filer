/********************************************************************************
** Form generated from reading UI file 'InfoDialogGWeimC.ui'
**
** Created by: Qt User Interface Compiler version 5.15.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef INFODIALOGGWEIMC_H
#define INFODIALOGGWEIMC_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>

QT_BEGIN_NAMESPACE

class Ui_InfoDialog
{
public:
    QGridLayout *gridLayout;
    QLabel *openWithInfo;
    QLabel *createdLabel;
    QPushButton *changeOpenWithButton;
    QLabel *modifiedLabel;
    QCheckBox *executableCheckBox;
    QLabel *pathLabel;
    QLabel *typeLabel_2;
    QLabel *permissionsInfo;
    QLabel *pathInfo;
    QLabel *typeInfo;
    QLabel *typeLabel;
    QLabel *sizeInfo;
    QLabel *iconLabel;
    QLabel *modifiedInfo;
    QLabel *iconInfo;
    QLabel *permissionsLabel;
    QLabel *sizeLabel;
    QSpacerItem *verticalSpacer;
    QLabel *createdInfo;

    void setupUi(QDialog *InfoDialog)
    {
        if (InfoDialog->objectName().isEmpty())
            InfoDialog->setObjectName(QString::fromUtf8("InfoDialog"));
        InfoDialog->resize(224, 371);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(InfoDialog->sizePolicy().hasHeightForWidth());
        InfoDialog->setSizePolicy(sizePolicy);
        gridLayout = new QGridLayout(InfoDialog);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        openWithInfo = new QLabel(InfoDialog);
        openWithInfo->setObjectName(QString::fromUtf8("openWithInfo"));
        QFont font;
        font.setPointSize(8);
        openWithInfo->setFont(font);

        gridLayout->addWidget(openWithInfo, 8, 1, 1, 1);

        createdLabel = new QLabel(InfoDialog);
        createdLabel->setObjectName(QString::fromUtf8("createdLabel"));
        createdLabel->setFont(font);
        createdLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(createdLabel, 3, 0, 1, 1);

        changeOpenWithButton = new QPushButton(InfoDialog);
        changeOpenWithButton->setObjectName(QString::fromUtf8("changeOpenWithButton"));
        QFont font1;
        font1.setPointSize(12);
        changeOpenWithButton->setFont(font1);

        gridLayout->addWidget(changeOpenWithButton, 9, 1, 1, 1);

        modifiedLabel = new QLabel(InfoDialog);
        modifiedLabel->setObjectName(QString::fromUtf8("modifiedLabel"));
        modifiedLabel->setFont(font);
        modifiedLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(modifiedLabel, 4, 0, 1, 1);

        executableCheckBox = new QCheckBox(InfoDialog);
        executableCheckBox->setObjectName(QString::fromUtf8("executableCheckBox"));
        executableCheckBox->setFont(font);

        gridLayout->addWidget(executableCheckBox, 6, 1, 1, 1);

        pathLabel = new QLabel(InfoDialog);
        pathLabel->setObjectName(QString::fromUtf8("pathLabel"));
        pathLabel->setFont(font);
        pathLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(pathLabel, 1, 0, 1, 1);

        typeLabel_2 = new QLabel(InfoDialog);
        typeLabel_2->setObjectName(QString::fromUtf8("typeLabel_2"));
        typeLabel_2->setFont(font);
        typeLabel_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(typeLabel_2, 8, 0, 1, 1);

        permissionsInfo = new QLabel(InfoDialog);
        permissionsInfo->setObjectName(QString::fromUtf8("permissionsInfo"));
        permissionsInfo->setFont(font);

        gridLayout->addWidget(permissionsInfo, 5, 1, 1, 1);

        pathInfo = new QLabel(InfoDialog);
        pathInfo->setObjectName(QString::fromUtf8("pathInfo"));
        pathInfo->setFont(font);

        gridLayout->addWidget(pathInfo, 1, 1, 1, 1);

        typeInfo = new QLabel(InfoDialog);
        typeInfo->setObjectName(QString::fromUtf8("typeInfo"));
        typeInfo->setFont(font);

        gridLayout->addWidget(typeInfo, 7, 1, 1, 1);

        typeLabel = new QLabel(InfoDialog);
        typeLabel->setObjectName(QString::fromUtf8("typeLabel"));
        typeLabel->setFont(font);
        typeLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(typeLabel, 7, 0, 1, 1);

        sizeInfo = new QLabel(InfoDialog);
        sizeInfo->setObjectName(QString::fromUtf8("sizeInfo"));
        sizeInfo->setFont(font);

        gridLayout->addWidget(sizeInfo, 2, 1, 1, 1);

        bool labelActive = false;

        iconLabel = new QLabel(InfoDialog);
        iconLabel->setObjectName(QString::fromUtf8("iconLabel"));
        iconLabel->setFont(font);
        iconLabel->setAlignment(Qt::AlignBottom|Qt::AlignRight|Qt::AlignTrailing);

        gridLayout->addWidget(iconLabel, 0, 0, 1, 1);

        modifiedInfo = new QLabel(InfoDialog);
        modifiedInfo->setObjectName(QString::fromUtf8("modifiedInfo"));
        modifiedInfo->setFont(font);

        gridLayout->addWidget(modifiedInfo, 4, 1, 1, 1);

        iconInfo = new QLabel(InfoDialog);
        iconInfo->setObjectName(QString::fromUtf8("iconInfo"));
        iconInfo->setMinimumSize(QSize(0, 128));
        iconInfo->setFont(font);

        gridLayout->addWidget(iconInfo, 0, 1, 1, 1);

        permissionsLabel = new QLabel(InfoDialog);
        permissionsLabel->setObjectName(QString::fromUtf8("permissionsLabel"));
        permissionsLabel->setFont(font);
        permissionsLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(permissionsLabel, 5, 0, 1, 1);

        sizeLabel = new QLabel(InfoDialog);
        sizeLabel->setObjectName(QString::fromUtf8("sizeLabel"));
        sizeLabel->setFont(font);
        sizeLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(sizeLabel, 2, 0, 1, 1);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer, 11, 1, 1, 1);

        createdInfo = new QLabel(InfoDialog);
        createdInfo->setObjectName(QString::fromUtf8("createdInfo"));
        createdInfo->setFont(font);

        gridLayout->addWidget(createdInfo, 3, 1, 1, 1);


        retranslateUi(InfoDialog);

        QMetaObject::connectSlotsByName(InfoDialog);
    } // setupUi

    void retranslateUi(QDialog *InfoDialog)
    {
        InfoDialog->setWindowTitle(QCoreApplication::translate("InfoDialog", "Dialog", nullptr));
        createdLabel->setText(QCoreApplication::translate("InfoDialog", "Created:", nullptr));
        changeOpenWithButton->setText(QCoreApplication::translate("InfoDialog", "Change...", nullptr));
        modifiedLabel->setText(QCoreApplication::translate("InfoDialog", "Modified:", nullptr));
        executableCheckBox->setText(QCoreApplication::translate("InfoDialog", "Excute as an application", nullptr));
        pathLabel->setText(QCoreApplication::translate("InfoDialog", "Path:", nullptr));
        typeLabel_2->setText(QCoreApplication::translate("InfoDialog", "Open With:", nullptr));
        typeLabel->setText(QCoreApplication::translate("InfoDialog", "Type:", nullptr));
        iconLabel->setText(QCoreApplication::translate("InfoDialog", "Icon:", nullptr));
        iconInfo->setText(QString());
        permissionsLabel->setText(QCoreApplication::translate("InfoDialog", "Permissions:", nullptr));
        sizeLabel->setText(QCoreApplication::translate("InfoDialog", "Size:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class InfoDialog: public Ui_InfoDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // INFODIALOGGWEIMC_H
