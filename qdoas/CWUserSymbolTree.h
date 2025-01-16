/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#ifndef _CWUSERSYMBOLTREE_H_GUARD
#define _CWUSERSYMBOLTREE_H_GUARD

#include <QTreeWidget>

#include "CWorkSpace.h"

class CWActiveContext;

class CWUserSymbolTree : public QTreeWidget, public CSymbolObserver
{
Q_OBJECT
 public:
 CWUserSymbolTree(CWActiveContext *activeContext, QWidget *parent = 0);
  virtual ~CWUserSymbolTree();

  void savePreferences(void);

  virtual void updateNewSymbol(const std::string &newSymbolName) override;
  virtual void updateModifySymbol(const std::string &symbolName) override;
  virtual void updateDeleteSymbol(const std::string &symbolName) override;

 protected:
  virtual void contextMenuEvent(QContextMenuEvent *e);
  virtual void showEvent(QShowEvent *e);

 private:
  void addNewUserSymbol(const QString &userSymbolName, const QString &description);

 public slots:
   void slotAddNewSymbol();
   void slotEditSymbol();
   void slotDeleteSymbol();

 signals:
  void signalWidthModeChanged(int newMode);

 private:
  CWActiveContext *m_activeContext;
};

#endif
