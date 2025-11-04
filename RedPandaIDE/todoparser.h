/*
 * Copyright (C) 2020-2022 Roy Qu (royqh1979@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef TODOPARSER_H
#define TODOPARSER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QAbstractListModel>
#include "syntaxermanager.h"
#include "qsynedit/constants.h"

/**
 * @brief Represents a TODO item found in source code
 * 
 * This structure holds information about a TODO or FIXME comment found
 * in the source code during parsing, including its location and content.
 */
struct TodoItem {
    QString filename;  //!< Name of the file containing the TODO item
    int lineNo;        //!< Line number where the TODO item is located
    int ch;            //!< Character position of the TODO item in the line
    QString line;      //!< The full content of the line containing the TODO item
};

using PTodoItem = std::shared_ptr<TodoItem>;

class TodoModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit TodoModel(QObject* parent=nullptr);
    void addItem(const QString& filename, int lineNo,
                 int ch, const QString& line);
    void removeTodosForFile(const QString& filename);
    
    /**
     * @brief Clear all TODO items in the current list (based on isForProject setting)
     */
    void clear();
    
    /**
     * @brief Clear TODO items in the specified list
     * @param forProject Whether to clear project TODO items (true) or general TODO items (false)
     */
    void clear(bool forProject);
    
    /**
     * @brief Get a TODO item at the specified model index
     * @param index Model index of the item to retrieve
     * @return Shared pointer to the TODO item, or null if index is invalid
     */
    PTodoItem getItem(const QModelIndex& index);
private:
    QList<PTodoItem> &getItems(bool forProject);
    const QList<PTodoItem> &getConstItems(bool forProject) const;
private:
    QList<PTodoItem> mItems;
    QList<PTodoItem> mProjectItems;
    bool mIsForProject;

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    int columnCount(const QModelIndex &parent) const override;
    bool isForProject() const;
    void setIsForProject(bool newIsForProject);

};

class TodoThread: public QThread
{
    Q_OBJECT
public:
    /**
     * @brief Constructor for parsing a single file
     * @param filename Name of the file to parse
     * @param parent Parent object
     */
    explicit TodoThread(const QString& filename, QObject* parent = nullptr);
    
    /**
     * @brief Constructor for parsing multiple files
     * @param files List of filenames to parse
     * @param parent Parent object
     */
    explicit TodoThread(const QStringList& files, QObject* parent = nullptr);

signals:
    /**
     * @brief Emitted when parsing starts
     */
    void parseStarted();
    
    /**
     * @brief Emitted when starting to parse a specific file
     * @param fileName Name of the file being parsed
     */
    void parsingFile(const QString& fileName);
    
    /**
     * @brief Emitted when a TODO item is found
     * @param filename Name of the file containing the TODO
     * @param lineNo Line number where the TODO was found
     * @param ch Character position of the TODO
     * @param line Content of the line containing the TODO
     */
    void todoFound(const QString& filename, int lineNo, int ch, const QString& line);
    
    /**
     * @brief Emitted when parsing is finished
     */
    void parseFinished();
private:
    /**
     * @brief Parse a single file
     */
    void parseFile();
    
    /**
     * @brief Parse multiple files
     */
    void parseFiles();
    void doParseFile(const QString& filename);
private:
    QString mFilename;
    QStringList mFiles;
    bool mParseFiles;

    // QThread interface
protected:
    /**
     * @brief Main thread execution function
     */
    void run() override;
};

using PTodoThread = std::shared_ptr<TodoThread>;

/**
 * @brief Main parser class for managing TODO parsing operations
 * 
 * This class manages the creation and execution of TodoThread objects
 * to parse source files for TODO and FIXME comments.
 */
class TodoParser : public QObject
{
    Q_OBJECT
public:
    explicit TodoParser(QObject *parent = nullptr);
    void parseFile(const QString& filename,bool isForProject);
    void parseFiles(const QStringList& files);
    
    /**
     * @brief Check if a parsing operation is currently in progress
     * @return True if parsing is in progress, false otherwise
     */
    bool parsing() const;

private:
    TodoThread* mThread;
    mutable QRecursiveMutex mMutex;
};

using PTodoParser = std::shared_ptr<TodoParser>;

#endif // TODOPARSER_H
