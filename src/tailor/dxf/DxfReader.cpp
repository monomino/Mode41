#include "DxfReader.h"
#include "DxfLineReader.h"
#include "DxfModel.h"
#include <QTextStream>
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <QException>
#include <QSharedPointer>

using namespace dxf;

DxfReader::DxfReader()
    : DxfLineReader()
{
}

void DxfReader::loadCore(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        throw QException();
    }
    setDevice(&file);
    reset();
    readAll();
    file.close();
}

void DxfReader::readAll()
{
    while (readZero())
    {
        Q_ASSERT(code() == 0);

        if (value() == "EOF")
        {
            break;
        }
        else if (value() == "SECTION")
        {
            readSECTION();
        }
        else if (value() == "ENDSEC")
        {
            readENDSEC();
        }
        else if (value() == "BLOCK")
        {
            readBLOCK();
        }
        else if (value() == "ENDBLK")
        {
            readENDBLK();
        }
        else if (value() == "INSERT")
        {
            readINSERT();
        }
        else if (value() == "TEXT")
        {
            readTEXT();
        }
        else if (value() == "LINE")
        {
            readLINE();
        }
        else if (value() == "POINT")
        {
            readPOINT();
        }
        else if (value() == "POLYLINE")
        {
            readPOLYLINE();
        }
        else if (value() == "VERTEX")
        {
            readVERTEX();
        }
        else if (value() == "SEQEND")
        {
            readSEQEND();
        }
        else
        {
            qDebug() << "unknown: " << value();
            skip();
        }
    }
}

/// 0
/// SECTION
/// 2
/// <SECTIONNAME>
/// ...
void DxfReader::readSECTION()
{
    Q_ASSERT(code() == 0);
    Q_ASSERT(value() == "SECTION");

    QString name;

    while (readNoZero())
    {
        if (code() == 2)
        {
            Q_ASSERT(code() == 2);
            name = value();
        }
    }

    addSECTION(name);
}

void DxfReader::readENDSEC()
{
    Q_ASSERT(code() == 0);
    Q_ASSERT(value() == "ENDSEC");

//    curr_section = "";
    skip();
    addENDSEC();
}

void DxfReader::readBLOCK()
{
    Q_ASSERT(code() == 0);
    Q_ASSERT(value() == "BLOCK");

    auto block = QSharedPointer<DxfBlock>::create();

    while (readNoZero())
    {
        if (code() == 8) // layer name
        {
            block->layer = value();
        }
        else if (code() == 2) // block name
        {
            block->name = value();
        }
        else if (code() == 70) // block flags
        {
            block->flags = value().toInt();
            // todo;
//            Block-type flags (bit coded values, may be combined):
//            1 = This is an anonymous block generated by hatching, associative dimensioning, other internal operations, or an application
//            2 = This block has attribute definitions
//            4 = This block is an external reference (xref)
//            8 = This block is an xref overlay
//            16 = This block is externally dependent
//            32 = This is a resolved external reference, or dependent of an external reference (ignored on input)
//            64 = This definition is a referenced external reference (ignored on input)
        }
        else if (code() == 10)
        {
            block->x = value().toDouble();
        }
        else if (code() == 20)
        {
            block->y = value().toDouble();
        }
        else if (code() == 30)
        {
            block->z = value().toDouble();
        }
    }

    addBLOCK(block);
}

void DxfReader::readENDBLK()
{
    skip();
    addENDBLK();
}

void DxfReader::readINSERT()
{
    auto insert = QSharedPointer<DxfInsert>::create();

    while (readNoZero())
    {
        if (code() == 2)
        {
            insert->block = value();
        }
        else if (code() == 8)
        {
            insert->layer = value();
        }
        else if (code() == 10) // x1
        {
            insert->x = value().toDouble();
        }
        else if (code() == 20) // y1
        {
            insert->y = value().toDouble();
        }
        else if (code() == 30) // z1
        {
            insert->z = value().toDouble();
        }
        else
        {
            qDebug() << "unhandled: " << code() << ", " << value() << " in " << Q_FUNC_INFO;
        }
    }

    addENTITY(insert);
}

void DxfReader::readTEXT()
{
    Q_ASSERT(code() == 0);
    Q_ASSERT(value() == "TEXT");

    auto text = QSharedPointer<DxfText>::create();

    while (readNoZero())
    {
        if (code() == 1)
        {
            text->text = value();
        }
        else if (code() == 7) // text style name, default = standard
        {

        }
        else if (code() == 8)
        {
            text->layer = value();
        }
        else if (code() == 10) // x1
        {
            text->x = value().toDouble();
        }
        else if (code() == 20) // y1
        {
            text->y = value().toDouble();
        }
        else if (code() == 30) // z1
        {
            text->z = value().toDouble();
        }
        else if (code() == 40) // size
        {
            text->size = value().toDouble();
        }
        else if (code() == 50) // rotation, default = 0
        {
            text->rotation = value().toDouble();
        }
        else
        {
            qDebug() << "unhandled: " << code() << ", " << value() << " in " << Q_FUNC_INFO;
        }
    }

    addENTITY(text);
}

void DxfReader::readLINE()
{
    Q_ASSERT(code() == 0);
    Q_ASSERT(value() == "LINE");

    auto line = QSharedPointer<DxfLine>::create();

    while (readNoZero())
    {
        Q_ASSERT(code() != 0);

        if (code() == 8) // layer name
        {
            line->layer = value();
        }
        else if (code() == 10) // x1
        {
            line->x1 = value().toDouble();
        }
        else if (code() == 20) // y1
        {
            line->y1 = value().toDouble();
        }
        else if (code() == 30) // z1
        {
            line->z1 = value().toDouble();
        }
        else if (code() == 11) // x2
        {
            line->x2 = value().toDouble();
        }
        else if (code() == 21) // y2
        {
            line->y2 = value().toDouble();
        }
        else if (code() == 31) // z2
        {
            line->z2 = value().toDouble();
        }
        else
        {
            qDebug() << "unhandled: " << code() << ", " << value() << " in " << Q_FUNC_INFO;
        }
    }

    addENTITY(line);
}

void DxfReader::readPOINT()
{
    Q_ASSERT(code() == 0);
    Q_ASSERT(value() == "POINT");

    auto point = QSharedPointer<DxfPoint>::create();

    while (readNoZero())
    {
        if (code() == 8) // layer name
        {
            point->layer = value();
        }
        else if (code() == 10) // x1
        {
            point->x = value().toDouble();
        }
        else if (code() == 20) // y1
        {
            point->y = value().toDouble();
        }
        else if (code() == 30) // z1
        {
            point->z = value().toDouble();
        }
        else if (code() == 38)
        {
            // todo: notch length
        }
        else if (code() == 39)
        {
            // todo: notch width
        }
        else if (code() == 50)
        {
            // todo: angle
        }
        else
        {
            qDebug() << "unhandled: " << code() << ", " << value() << " in " << Q_FUNC_INFO;
        }
    }

    addENTITY(point);
}

void DxfReader::readPOLYLINE()
{
    Q_ASSERT(code() == 0);
    Q_ASSERT(value() == "POLYLINE");

    auto polyline = QSharedPointer<DxfPolyline>::create();

    while (readNoZero())
    {
        if (code() == 66)
        {
            /// \todo polyline attribte group code = 66
        }
        else if (code() == 8)
        {
            polyline->layer = value();
        }
        else if (code() == 70)
        {
    //            Polyline flag (bit-coded); default is 0:
    //            1 = This is a closed polyline (or a polygon mesh closed in the
    //            M direction).
    //            2 = Curve-fit vertices have been added.
    //            4 = Spline-fit vertices have been added.
    //            8 = This is a 3D polyline.
    //            16 = This is a 3D polygon mesh.
    //            32 = The polygon mesh is closed in the N direction.
    //            64 = The polyline is a polyface mesh.
    //            128 = The linetype pattern is generated continuously around the vertices of this polyline.
            polyline->flags = value().toInt();
        }
        else
        {
            qDebug() << "unhandled: " << code() << ", " << value() << " in " << Q_FUNC_INFO;
        }
    }

    //last_polyline = entity;
    addENTITY(polyline);
}

void DxfReader::readVERTEX()
{
    Q_ASSERT(code() == 0);
    Q_ASSERT(value() == "VERTEX");

    auto vertex = QSharedPointer<DxfVertex>::create();

    while (readNoZero())
    {
        if (code() == 8)
        {
            vertex->layer = value();
        }
        else if (code() == 10)
        {
            vertex->x = value().toDouble();
        }
        else if (code() == 20)
        {
            vertex->y = value().toDouble();
        }
        else if (code() == 70)
        {
            vertex->flags = value().toInt();
        }
    }

    addVERTEX(vertex);
}

void DxfReader::readSEQEND()
{
    Q_ASSERT(code() == 0);
    Q_ASSERT(value() == "SEQEND");

    skip();
    addSEQEND();
}

