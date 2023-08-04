/********************************************************************************
** Form generated from reading UI file 'InfoDialogNVFNVO.ui'
**
** Created by: Qt User Interface Compiler version 5.15.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef INFODIALOGNVFNVO_H
#define INFODIALOGNVFNVO_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>

QT_BEGIN_NAMESPACE

class Ui_InfoDialog
{
public:
    QGridLayout *gridLayout;
    QLabel *permissionsInfo;
    QLabel *modifiedInfo;
    QLabel *permissionsLabel;
    QLabel *iconLabel;
    QLabel *typeInfo;
    QLabel *modifiedLabel;
    QLabel *iconInfo;
    QLabel *createdInfo;
    QLabel *sizeLabel;
    QLabel *createdLabel;
    QLabel *sizeInfo;
    QLabel *pathInfo;
    QLabel *pathLabel;
    QLabel *typeLabel;
    QSpacerItem *verticalSpacer;
    QSpacerItem *horizontalSpacer;

    void setupUi(QDialog *InfoDialog)
    {
        if (InfoDialog->objectName().isEmpty())
            InfoDialog->setObjectName(QString::fromUtf8("InfoDialog"));
        InfoDialog->resize(400, 466);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(InfoDialog->sizePolicy().hasHeightForWidth());
        InfoDialog->setSizePolicy(sizePolicy);
        gridLayout = new QGridLayout(InfoDialog);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        permissionsInfo = new QLabel(InfoDialog);
        permissionsInfo->setObjectName(QString::fromUtf8("permissionsInfo"));
        QFont font;
        font.setPointSize(8);
        permissionsInfo->setFont(font);

        gridLayout->addWidget(permissionsInfo, 5, 2, 1, 1);

        modifiedInfo = new QLabel(InfoDialog);
        modifiedInfo->setObjectName(QString::fromUtf8("modifiedInfo"));
        modifiedInfo->setFont(font);

        gridLayout->addWidget(modifiedInfo, 4, 2, 1, 1);

        permissionsLabel = new QLabel(InfoDialog);
        permissionsLabel->setObjectName(QString::fromUtf8("permissionsLabel"));
        permissionsLabel->setFont(font);
        permissionsLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(permissionsLabel, 5, 0, 1, 1);

        iconLabel = new QLabel(InfoDialog);
        iconLabel->setObjectName(QString::fromUtf8("iconLabel"));
        iconLabel->setFont(font);
        iconLabel->setAlignment(Qt::AlignBottom|Qt::AlignRight|Qt::AlignTrailing);

        gridLayout->addWidget(iconLabel, 0, 0, 1, 1);

        typeInfo = new QLabel(InfoDialog);
        typeInfo->setObjectName(QString::fromUtf8("typeInfo"));
        typeInfo->setFont(font);

        gridLayout->addWidget(typeInfo, 6, 2, 1, 1);

        modifiedLabel = new QLabel(InfoDialog);
        modifiedLabel->setObjectName(QString::fromUtf8("modifiedLabel"));
        modifiedLabel->setFont(font);
        modifiedLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(modifiedLabel, 4, 0, 1, 1);

        iconInfo = new QLabel(InfoDialog);
        iconInfo->setObjectName(QString::fromUtf8("iconInfo"));
        iconInfo->setMinimumSize(QSize(0, 128));
        iconInfo->setFont(font);

        gridLayout->addWidget(iconInfo, 0, 2, 1, 1);

        createdInfo = new QLabel(InfoDialog);
        createdInfo->setObjectName(QString::fromUtf8("createdInfo"));
        createdInfo->setFont(font);

        gridLayout->addWidget(createdInfo, 3, 2, 1, 1);

        sizeLabel = new QLabel(InfoDialog);
        sizeLabel->setObjectName(QString::fromUtf8("sizeLabel"));
        sizeLabel->setFont(font);
        sizeLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(sizeLabel, 2, 0, 1, 1);

        createdLabel = new QLabel(InfoDialog);
        createdLabel->setObjectName(QString::fromUtf8("createdLabel"));
        createdLabel->setFont(font);
        createdLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(createdLabel, 3, 0, 1, 1);

        sizeInfo = new QLabel(InfoDialog);
        sizeInfo->setObjectName(QString::fromUtf8("sizeInfo"));
        sizeInfo->setFont(font);

        gridLayout->addWidget(sizeInfo, 2, 2, 1, 1);

        pathInfo = new QLabel(InfoDialog);
        pathInfo->setObjectName(QString::fromUtf8("pathInfo"));
        pathInfo->setFont(font);

        gridLayout->addWidget(pathInfo, 1, 2, 1, 1);

        pathLabel = new QLabel(InfoDialog);
        pathLabel->setObjectName(QString::fromUtf8("pathLabel"));
        pathLabel->setFont(font);
        pathLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(pathLabel, 1, 0, 1, 1);

        typeLabel = new QLabel(InfoDialog);
        typeLabel->setObjectName(QString::fromUtf8("typeLabel"));
        typeLabel->setFont(font);
        typeLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(typeLabel, 6, 0, 1, 1);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer, 7, 2, 1, 1);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer, 2, 3, 1, 1);


        retranslateUi(InfoDialog);

        QMetaObject::connectSlotsByName(InfoDialog);
    } // setupUi

    void retranslateUi(QDialog *InfoDialog)
    {
        InfoDialog->setWindowTitle(QCoreApplication::translate("InfoDialog", "Dialog", nullptr));
        permissionsLabel->setText(QCoreApplication::translate("InfoDialog", "Permissions:", nullptr));
        iconLabel->setText(QCoreApplication::translate("InfoDialog", "Icon:", nullptr));
        modifiedLabel->setText(QCoreApplication::translate("InfoDialog", "Modified:", nullptr));
        iconInfo->setText(QString());
        sizeLabel->setText(QCoreApplication::translate("InfoDialog", "Size:", nullptr));
        createdLabel->setText(QCoreApplication::translate("InfoDialog", "Created:", nullptr));
        pathLabel->setText(QCoreApplication::translate("InfoDialog", "Path:", nullptr));
        typeLabel->setText(QCoreApplication::translate("InfoDialog", "Type:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class InfoDialog: public Ui_InfoDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // INFODIALOGNVFNVO_H
