#include "files.h"
#include "base.h"

#ifdef CODEBASE_FEATURE_LICENSE_EMBEDDMENT_API
#include <licensing/licensing.h>
#endif // ~CODEBASE_FEATURE_LICENSE_EMBEDDMENT_API

#include <fstream>
#include <sstream>
#include <type_traits>

#ifndef DISABLE_UBJSON_FORMAT
#include <ubjsoncpp/include/stream_reader.hpp>
#include <ubjsoncpp/include/stream_writer.hpp>
#endif

#include <rapidjson/include/rapidjson/document.h>
#include <rapidjson/include/rapidjson/reader.h>
#include <rapidjson/include/rapidjson/stringbuffer.h>
#include <rapidjson/include/rapidjson/istreamwrapper.h>
#include <rapidjson/include/rapidjson/writer.h>
#include <rapidjson/include/rapidjson/prettywriter.h>

#include <miniz/miniz.h>
#include <miniz/miniz_zip.h>

#include <QtGui/private/qzipwriter_p.h>
#include <QtGui/private/qzipreader_p.h>

#include <QFile>
#include <QBuffer>

namespace JsonUtils
{
  namespace Files
  {
    FormatFlags ResolveFormat(const QString& fileName, FormatFlags f)
    {
      if (f == FormatAuto)
      {
        if (fileName.endsWith(".cfrac",   Qt::CaseInsensitive)  // project binary
         || fileName.endsWith(".cfracdb", Qt::CaseInsensitive)  // user databases binary
         || fileName.endsWith(".ac",      Qt::CaseInsensitive)  // acids database binary
         || fileName.endsWith(".fl",      Qt::CaseInsensitive)  // fluids database binary
         || fileName.endsWith(".pr",      Qt::CaseInsensitive)  // proppants database binary
         )
          return FormatCompressed;
        if (fileName.endsWith(".json", Qt::CaseInsensitive))
          return FormatPretty;
         if (fileName.endsWith(".ubj", Qt::CaseInsensitive))
          return FormatBinary;
      }
       if (f == FormatPretty)
      {
        return FormatPretty;
      }
       if (f == FormatBinary)
      {
        return FormatBinary;
      }
      return FormatCompressed; // <- default to binary proprietary
    }

    FormatFlags ResolveFormat(const QString& fileName)
    {
      return ResolveFormat(fileName, FormatAuto);
    }

    namespace
    {
      namespace Z
      {
        struct LocalFileHeaderZ
        {
          uchar signature[4]; //  0x04034b50
          uchar version_needed[2];
          uchar general_purpose_bits[2];
          uchar compression_method[2];
          uchar last_mod_file[4];
          uchar crc_32[4];
          uchar compressed_size[4];
          uchar uncompressed_size[4];
          uchar file_name_length[2];
          uchar extra_field_length[2];
        };

        static inline uint readUInt(const uchar *data)
        {
          return (data[0]) + (data[1] << 8) + (data[2] << 16) + (data[3] << 24);
        }
      }

      class RapidJsonStringBufferDevice : public QIODevice
      {
        public:
          explicit RapidJsonStringBufferDevice(rapidjson::StringBuffer& underlying)
            : QIODevice()
            , Underlying(underlying)
            , Offset(0)
          {
          }

          ~RapidJsonStringBufferDevice()
          {
          }

          qint64 size() const override
          {
            return Underlying.GetSize();
          }

          virtual qint64 readData(char *data, qint64 maxSize) override
          {
            qint64 limit = Underlying.GetSize() - Offset;
            qint64 read = maxSize;
            if (read > limit)
              read = limit;

            memcpy(data, Underlying.GetString() + Offset, read);
            Offset += read;
            return read;
          }

          virtual qint64 writeData(const char *data, qint64 maxSize) override
          {
            for (qint64 i = 0; i < maxSize; ++i)
            {
              Underlying.Put(data[i]);
            }
            return maxSize;
          }

          virtual bool seek(qint64 pos) override
          {
            Offset = pos;
            return QIODevice::seek(pos);
          }

        private:
          rapidjson::StringBuffer& Underlying;
          qint64                   Offset;
      };

      std::string soSecureString = "so cfrac, such secure";

      void Salt(QByteArray& data)
      {
        auto it = soSecureString.begin();
        char* saltedData = data.data();
        for (int i = 0; i < data.size(); ++i)
        {
          saltedData[i] ^= *it;
          ++it;
          if (it == soSecureString.end())
            it = soSecureString.begin();
        }
      }

      void Salt(rapidjson::StringBuffer& buffer)
      {
        auto it = soSecureString.begin();
        char* saltedData = const_cast<char*>(buffer.GetString());
        for (qint64 i = 0; i < buffer.GetSize(); ++i)
        {
          saltedData[i] ^= *it;
          ++it;
          if (it == soSecureString.end())
            it = soSecureString.begin();
        }
      }

      void WriteCompressedFile(QByteArray& out, const QByteArray &data)
      {
        QByteArray salted;
        QBuffer saltedBuffer(&salted);
        saltedBuffer.open(QIODevice::WriteOnly);

        QZipWriter zip(&saltedBuffer);
        zip.setCompressionPolicy(QZipWriter::AlwaysCompress);
        zip.addFile("frac_project", data);
        zip.close();
        saltedBuffer.close();

        Salt(salted);
        out = salted;
      }

      void WriteCompressedFile(const QString &fileName, const QByteArray &data)
      {
        QByteArray compressed;
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly))
        {
          WriteCompressedFile(compressed, data);
          file.write(compressed);
          file.close();
        }
      }

      void WriteCompressedFile(const QString &fileName, rapidjson::StringBuffer& buffer)
      {
        RapidJsonStringBufferDevice bufferDevice(buffer);

        rapidjson::StringBuffer zipped;
        RapidJsonStringBufferDevice zippedDevice(zipped);

        QZipWriter zip(&zippedDevice);
        zip.setCompressionPolicy(QZipWriter::AlwaysCompress);
        zip.addFile("frac_project", &bufferDevice);
        zip.close();

        Salt(zipped);
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly))
        {
          file.write(zipped.GetString(), zipped.GetSize());
        }
      }

      void ReadCompressedFile(const QByteArray& compressedData, QByteArray& data)
      {
        QByteArray salted = compressedData;
        Salt(salted);

        QBuffer saltedBuffer(&salted);
        saltedBuffer.open(QIODevice::ReadOnly);

        QZipReader zip(&saltedBuffer);
        if (zip.exists())
        {
          data = zip.fileData("frac_project");
          zip.close();
        }
        saltedBuffer.close();
      }

      void ReadCompressedFile(const QString& fileName, QByteArray& data)
      {
        QByteArray compressedData;
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly))
        {
          compressedData = file.readAll();
          file.close();

          ReadCompressedFile(compressedData, data);
        }
      }

      void ReadCompressedFile(const QString &fileName, rapidjson::StringBuffer& out)
      {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly))
        {
          char* data = new char[file.size() + 1];

#ifdef CODEBASE_FEATURE_LICENSE_EMBEDDMENT_API
          CET_LICENSE_CHECK_OR_INVALIDATE_PTR(data);
#endif // ~CODEBASE_FEATURE_LICENSE_EMBEDDMENT_API

          file.read(data, file.size());
          data[file.size()] = 0;

          rapidjson::StringBuffer compressed;
          rapidjson::PutN(compressed, 0, file.size());
          memcpy(const_cast<char*>(compressed.GetString()), data, file.size());

          Salt(compressed);

          mz_zip_archive archive = { 0 };
          if (mz_zip_reader_init_mem(&archive, compressed.GetString(), compressed.GetSize(), 0))
          {
            if (mz_zip_reader_locate_file(&archive, "frac_project", NULL, MZ_ZIP_FLAG_IGNORE_PATH) >= 0)
            {
              mz_zip_archive_file_stat file_stat = { 0 };
              if (mz_zip_reader_file_stat(&archive, 0, &file_stat))
              {
                size_t uncompressed_size = file_stat.m_uncomp_size;
                if (uncompressed_size)
                {
                  void* p = mz_zip_reader_extract_file_to_heap(&archive, file_stat.m_filename, &uncompressed_size, 0);
                  if (p)
                  {
                    out.Clear();
                    rapidjson::PutN(out, 0, uncompressed_size);
                    memcpy(const_cast<char*>(out.GetString()), p, uncompressed_size);

                    mz_free(p);
                  }
                }
              }
            }
          }
          delete[] data;
        }
      }
    }

    void WriteJsonFile(QByteArray& data, const QJsonValue& value, FormatFlags f)
    {
      if (f == FormatBinary)
      {
        // @todo
      }
      else if (f == FormatPretty)
      {
        QJsonDocument document;
        if (value.isObject())
        {
          document.setObject(value.toObject());
        }
        else if (value.isArray())
        {
          document.setArray(value.toArray());
        }
        data = document.toJson();
      }
      else if (f == FormatCompressed)
      {
        QJsonDocument document;
        if (value.isObject())
        {
          document.setObject(value.toObject());
        }
        else if (value.isArray())
        {
          document.setArray(value.toArray());
        }
        data = document.toJson();
        WriteCompressedFile(data, document.toJson());
      }
    }

    void WriteJsonFile(const QString& fileName, const QJsonDocument& document, FormatFlags f)
    {
      f = ResolveFormat(fileName, f);
      if (f == FormatBinary)
      {
#ifndef DISABLE_UBJSON_FORMAT
        ubjson::Value ubjValue;
        Convert::QJsonToUbjson(document.object(), ubjValue);
        return WriteJsonFile(fileName, ubjValue, f);
#endif
      }
      if (f == FormatPretty)
      {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly))
        {
          file.write(document.toJson());
        }
      }
      if (f == FormatCompressed)
      {
        WriteCompressedFile(fileName, document.toJson());
      }
    }

    void WriteJsonFile(const QString& fileName, const QJsonValue& value, FormatFlags f)
    {
      f = ResolveFormat(fileName, f);
      if (f == FormatBinary)
      {
#ifndef DISABLE_UBJSON_FORMAT
        ubjson::Value ubjValue;
        Convert::QJsonToUbjson(value, ubjValue);
        return WriteJsonFile(fileName, ubjValue, f);
#endif
      }
      if (f == FormatPretty)
      {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly))
        {
          QJsonDocument document(value.toObject());
          file.write(document.toJson());
        }
      }
      if (f == FormatCompressed)
      {
        QJsonDocument document(value.toObject());
        WriteCompressedFile(fileName, document.toJson());
      }
    }

#ifndef DISABLE_UBJSON_FORMAT
    void WriteJsonFile(const QString& fileName, const ubjson::Value& value, FormatFlags f)
    {
      f = ResolveFormat(fileName, f);
      if (f == FormatPretty)
      {
        QJsonValue qtJson;
        Convert::UbjsonToQJson(value, qtJson);
        return WriteJsonFile(fileName, qtJson, f);
      }
      /*if (f == FormatBinary)*/
      {
        std::ofstream file;
        file.open(fileName.toStdString(), std::ios::binary);
        ubjson::StreamWriter<decltype(file)> writer(file);
        writer.writeValue(value);
      }
    }
#endif

    void WriteJsonFile(const QString& fileName, const rapidjson::Value& value, FormatFlags f)
    {
      rapidjson::StringBuffer buffer;
      rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
      value.Accept(writer);

      f = ResolveFormat(fileName, f);
      if (f == FormatPretty)
      {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly))
        {
          file.write(buffer.GetString(), buffer.GetSize());
        }
      }
      else if (f == FormatCompressed)
      {
        WriteCompressedFile(fileName, buffer);
      }
    }

    void ReadJsonFile(const QByteArray& data, QJsonValue& value, FormatFlags f)
    {
      if (f == FormatBinary)
      {
        // @todo
      }
      else if (f == FormatPretty)
      {
        QJsonParseError error;
        QJsonDocument document;
        document = QJsonDocument::fromJson(data, &error);
        value = document.object();
      }
      else if (f == FormatCompressed)
      {
        QByteArray dataJson;
        ReadCompressedFile(data, dataJson);
        QJsonParseError error;
        QJsonDocument document;
        document = QJsonDocument::fromJson(dataJson, &error);
        value = document.object();
      }
    }

    void ReadJsonFile(const QString& fileName, QJsonDocument& document, FormatFlags f)
    {
      f = ResolveFormat(fileName, f);
      if (f == FormatBinary)
      {
#ifndef DISABLE_UBJSON_FORMAT
        ubjson::Value ubjValue;
        ReadJsonFile(fileName, ubjValue, f);
        document = Convert::UbjsonToQJsonDocument(ubjValue);
#endif
      }
      else if (f == FormatPretty)
      {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly))
        {
          QByteArray data = file.readAll();
          QJsonParseError error;
          document = QJsonDocument::fromJson(data, &error);
        }
      }
      else if (f == FormatCompressed)
      {
        QByteArray data;
        ReadCompressedFile(fileName, data);
        QJsonParseError error;
        document = QJsonDocument::fromJson(data, &error);
      }
    }

    void ReadJsonFile(const QString& fileName, QJsonValue& value, FormatFlags f)
    {
      f = ResolveFormat(fileName, f);
      if (f == FormatBinary)
      {
#ifndef DISABLE_UBJSON_FORMAT
        ubjson::Value ubjValue;
        ReadJsonFile(fileName, ubjValue, f);
        Convert::UbjsonToQJson(ubjValue, value);
#endif
      }
      else if (f == FormatPretty)
      {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly))
        {
          QByteArray data = file.readAll();
          QJsonParseError error;
          QJsonDocument document(QJsonDocument::fromJson(data, &error));
          value = document.object();
        }
      }
      else if (f == FormatCompressed)
      {
        QByteArray data;
        ReadCompressedFile(fileName, data);
        QJsonParseError error;
        QJsonDocument document(QJsonDocument::fromJson(data, &error));
        value = document.object();
      }
    }

#ifndef DISABLE_UBJSON_FORMAT
    void ReadJsonFile(const QString& fileName, ubjson::Value& value, FormatFlags f)
    {
      f = ResolveFormat(fileName, f);
      if (f == FormatPretty)
      {
        QJsonValue qtValue;
        ReadJsonFile(fileName, qtValue, f);
        Convert::QJsonToUbjson(qtValue, value);
      }
      else /*if (f == FormatBinary)*/
      {
        std::ifstream file;
        file.open(fileName.toStdString(), std::ios::binary);
        ubjson::StreamReader<decltype(file)> reader(file);
        reader.getNextValue(value);
      }
    }
#endif

    void ReadJsonFile(const QString& fileName, rapidjson::Document& document, FormatFlags f)
    {
      f = ResolveFormat(fileName, f);

      if (f == FormatPretty)
      {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly))
        {
          rapidjson::StringBuffer buffer;
          RapidJsonStringBufferDevice bufferDevice(buffer);

          rapidjson::PutN(buffer, 0, file.size());

          file.read(const_cast<char*>(buffer.GetString()), buffer.GetSize());

          rapidjson::StringStream stream(buffer.GetString());
          document.ParseStream(stream);
        }
      }
      else if (f == FormatCompressed)
      {
        rapidjson::StringBuffer buffer;
        ReadCompressedFile(fileName, buffer);

        rapidjson::StringStream stream(buffer.GetString());
        document.ParseStream(stream);
      }
    }
  }
}
