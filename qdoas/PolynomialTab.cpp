#include "PolynomialTab.h"

#include <QLabel>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QCheckBox>

#include "constants.h"
#include "mediate_general.h"

//#include <iostream>
//using std::cout; using std::endl;

// Functions header_label, white_frame, add_layout and add_widget are
// there to imitate the layout of the other Analysis window tabs
// (defined in CDoasTable)

static QLabel *header_label(const QString& text, QWidget *parent = NULL) {
  QLabel *label = new QLabel(text, parent);
  label->setLineWidth(2);
  label->setFrameStyle(QFrame::Panel | QFrame::Raised);
  return label;
}

// wrap all widgets in a white background frame, with a margin of one
// pixel
static QFrame *white_frame() {
  QFrame *frame = new QFrame;
  QPalette pal(frame->palette());
  pal.setColor(QPalette::Window, Qt::white);
  frame->setAutoFillBackground(true);
  frame->setPalette(pal);
  return frame;
}

static void add_layout(QGridLayout *grid_layout, QLayout *layout, int row, int col) {
  QFrame *frame = white_frame();
  frame->setLayout(layout);
  grid_layout->addWidget(frame, row, col);
}

static void add_widget(QGridLayout *grid_layout, QWidget *w, int row, int col, Qt::Alignment align = Qt::AlignCenter) {
  QHBoxLayout *layout = new QHBoxLayout;
  layout->addWidget(w, 1, align);
  add_layout(grid_layout, layout, row, col);
}

PolynomialTab::PolynomialTab(QWidget *parent) :
  QFrame(parent) {

  store_fit_poly = new QCheckBox(this);
  store_fit_offset = new QCheckBox(this);
  store_err_poly = new QCheckBox(this);
  store_err_offset = new QCheckBox(this);
  offset_izero = new QCheckBox(this);
  poly_order = new QComboBox(this);
  offset_order = new QComboBox(this);
  orthobase_order = new QComboBox(this);

  QGridLayout *layout = new QGridLayout(this);
  layout->setSpacing(0);

  layout->addWidget(header_label("Polynomial"), 1, 0);
  layout->addWidget(header_label("Linear Offset"), 2, 0);

  layout->addWidget(header_label("Order"), 0, 1);
  layout->addWidget(header_label("Fit Store"), 0, 2);
  layout->addWidget(header_label("Err Store"), 0, 3);
  layout->addWidget(header_label("Specific"), 0, 4);

  add_widget(layout, store_fit_poly, 1, 2, Qt::AlignCenter);
  add_widget(layout, store_fit_offset, 2, 2, Qt::AlignCenter);
  add_widget(layout, store_err_poly, 1, 3, Qt::AlignCenter);
  add_widget(layout, store_err_offset, 2, 3, Qt::AlignCenter);

  QStringList comboItems;
  comboItems << "None" << "Order 0" << "Order 1" << "Order 2" << "Order 3"
             << "Order 4" << "Order 5" << "Order 6" << "Order 7" << "Order 8";
  poly_order->addItems(comboItems);
  offset_order->addItems(comboItems);
  add_widget(layout, poly_order, 1, 1);
  add_widget(layout, offset_order, 2, 1);

  QHBoxLayout *orthobase_layout = new QHBoxLayout;
  orthobase_layout->addWidget(new QLabel("Orthobase Order", this), Qt::AlignRight);
  orthobase_order->addItems(comboItems);
  orthobase_layout->addWidget(orthobase_order, Qt::AlignLeft);
  add_layout(layout, orthobase_layout, 1, 4);

  QHBoxLayout *izero_layout = new QHBoxLayout;
  izero_layout->addWidget(offset_izero);
  izero_layout->addWidget(new QLabel("apply to reference", this), Qt::AlignLeft);
  add_layout(layout, izero_layout, 2, 4);

  layout->addWidget(white_frame(), 3, 1, 1, 4);
  layout->setRowStretch(3,1);
  layout->setColumnStretch(6,1);
}

void PolynomialTab::apply(struct anlyswin_linear *data) const {
  data->xPolyOrder = poly_order->currentIndex();
  data->xBaseOrder = orthobase_order->currentIndex();
  data->xFlagFitStore = store_fit_poly->isChecked();
  data->xFlagErrStore = store_err_poly->isChecked();

  data->offsetPolyOrder = offset_order->currentIndex();
  data->offsetFlagFitStore = store_fit_offset->isChecked();
  data->offsetFlagErrStore = store_err_offset->isChecked();

  data->offsetI0 = offset_izero->isChecked();

//  cout << " xPolyOrder: " << data->xPolyOrder
//       << " xBaseOrder: " << data->xBaseOrder
//       << " xFlagFitStore: " << data->xFlagFitStore
//       << " offsetPolyOrder: " <<data->offsetPolyOrder
//       << " offsetFlagFitStore: " << data->offsetFlagFitStore
//       << " offsetFlagErrStore: " << data->offsetFlagErrStore
//       << " offsetI0: " << data->offsetI0 << endl;
}

void PolynomialTab::populate(const struct anlyswin_linear *data) {
  poly_order->setCurrentIndex(data->xPolyOrder);
  orthobase_order->setCurrentIndex(data->xBaseOrder);
  store_fit_poly->setChecked(data->xFlagFitStore);
  store_err_poly->setChecked(data->xFlagErrStore);

  offset_order->setCurrentIndex(data->offsetPolyOrder);
  store_fit_offset->setChecked(data->offsetFlagFitStore);
  store_err_offset->setChecked(data->offsetFlagErrStore);

  offset_izero->setChecked(data->offsetI0);
}
