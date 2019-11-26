#ifndef POLYNOMIALTAB_GUARD_H
#define POLYNOMIALTAB_GUARD_H

#include <QFrame>

class QCheckBox;
class QComboBox;

class PolynomialTab : public QFrame
{
Q_OBJECT
public:

  PolynomialTab(QWidget *parent = NULL);

  void populate(const struct anlyswin_linear *data);
  void apply(struct anlyswin_linear *data) const;

private:
  QCheckBox *store_fit_poly, *store_fit_offset,
    *store_err_poly, *store_err_offset, *offset_izero;
  QComboBox *poly_order, *offset_order, *orthobase_order;
};

#endif
