#include "qb_arm_rviz_plugins/trigger_gemini_panel.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <rviz_common/display_context.hpp>
#include <thread>
#include <chrono>
#include <set>
#include <QScrollBar>

namespace qb_arm_rviz_plugins
{

TriggerGeminiPanel::TriggerGeminiPanel(QWidget * parent)
: rviz_common::Panel(parent), target_class_str_("scissors")
{
  QVBoxLayout* layout = new QVBoxLayout;
  
  QHBoxLayout* input_layout = new QHBoxLayout;
  input_layout->addWidget(new QLabel("Target Class:"));
  target_class_editor_ = new QLineEdit(target_class_str_);
  input_layout->addWidget(target_class_editor_);
  layout->addLayout(input_layout);

  button_ = new QPushButton("Trigger");
  layout->addWidget(button_);

  QHBoxLayout* log_header_layout = new QHBoxLayout;
  log_header_layout->addWidget(new QLabel("Log Node:"));
  node_selector_ = new QComboBox();
  node_selector_->setEditable(true);
  node_selector_->addItem("gemini_pick");
  log_header_layout->addWidget(node_selector_);

  info_cb_ = new QCheckBox("Info");
  warn_cb_ = new QCheckBox("Warnings");
  error_cb_ = new QCheckBox("Errors");
  info_cb_->setChecked(true);
  warn_cb_->setChecked(true);
  error_cb_->setChecked(true);

  log_header_layout->addWidget(info_cb_);
  log_header_layout->addWidget(warn_cb_);
  log_header_layout->addWidget(error_cb_);
  
  layout->addLayout(log_header_layout);

  log_display_ = new QTextEdit();
  log_display_->setReadOnly(true);
  log_display_->setLineWrapMode(QTextEdit::WidgetWidth);
  log_display_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  log_display_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  layout->addWidget(log_display_);
  
  setLayout(layout);

  connect(button_, SIGNAL(clicked()), this, SLOT(onButtonClick()));
  connect(target_class_editor_, SIGNAL(editingFinished()), this, SLOT(updateTargetClass()));
  connect(node_selector_, SIGNAL(currentIndexChanged(int)), this, SLOT(onNodeSelected(int)));
  connect(node_selector_, SIGNAL(editTextChanged(const QString &)), this, SLOT(refreshLogDisplay()));
  connect(info_cb_, SIGNAL(stateChanged(int)), this, SLOT(onLevelFilterChanged(int)));
  connect(warn_cb_, SIGNAL(stateChanged(int)), this, SLOT(onLevelFilterChanged(int)));
  connect(error_cb_, SIGNAL(stateChanged(int)), this, SLOT(onLevelFilterChanged(int)));

  discovery_timer_ = new QTimer(this);
  connect(discovery_timer_, SIGNAL(timeout()), this, SLOT(updateNodeList()));
}

TriggerGeminiPanel::~TriggerGeminiPanel()
{
}

void TriggerGeminiPanel::onInitialize()
{
  node_ = getDisplayContext()->getRosNodeAbstraction().lock()->get_raw_node();
  
  // Publisher for target class
  publisher_ = node_->create_publisher<std_msgs::msg::String>("/target_class", 10);
  
  // Client for trigger service (Hardcoded)
  client_ = node_->create_client<std_srvs::srv::Trigger>("/trigger_gemini_pick");

  // Subscribe to /rosout
  log_sub_ = node_->create_subscription<rcl_interfaces::msg::Log>(
    "/rosout", 10,
    [this](const rcl_interfaces::msg::Log::SharedPtr msg) {
      QString node_name = QString::fromStdString(msg->name);
      QString qmsg = QString::fromStdString(msg->msg);
      int level = msg->level;

      // Store in history
      LogMessage entry = {level, node_name, qmsg};
      all_logs_.push_back(entry);
      if (all_logs_.size() > max_logs_) {
        all_logs_.pop_front();
      }

      // Check if it matches current selection and level filter
      if (node_name == node_selector_->currentText()) {
        bool show = false;
        if (level == rcl_interfaces::msg::Log::INFO && info_cb_->isChecked()) show = true;
        if (level == rcl_interfaces::msg::Log::WARN && warn_cb_->isChecked()) show = true;
        if ((level == rcl_interfaces::msg::Log::ERROR || level == rcl_interfaces::msg::Log::FATAL) && error_cb_->isChecked()) show = true;

        if (show) {
          QMetaObject::invokeMethod(this, "appendLog", Qt::QueuedConnection,
                                    Q_ARG(int, level),
                                    Q_ARG(QString, qmsg));
        }
      }
    });

  discovery_timer_->start(2000); // Update node list every 2 seconds
}

void TriggerGeminiPanel::onLevelFilterChanged(int state)
{
  (void)state;
  refreshLogDisplay();
}

void TriggerGeminiPanel::updateNodeList()
{
  if (!node_) return;
  
  auto node_names = node_->get_node_names();
  QString current = node_selector_->currentText();

  // Temporary block signals to avoid triggering onNodeSelected repeatedly
  node_selector_->blockSignals(true);
  
  // Get all unique names from logs + current nodes
  std::set<QString> names;
  names.insert("gemini_pick");
  for (const auto& n : node_names) {
    QString name = QString::fromStdString(n);
    if (name.startsWith("/")) name = name.mid(1);
    names.insert(name);
  }
  for (const auto& log : all_logs_) {
    names.insert(log.name);
  }

  // Update combo box
  node_selector_->clear();
  for (const auto& name : names) {
    node_selector_->addItem(name);
  }
  
  int index = node_selector_->findText(current);
  if (index >= 0) {
    node_selector_->setCurrentIndex(index);
  } else {
    node_selector_->setEditText(current);
  }

  node_selector_->blockSignals(false);
}

void TriggerGeminiPanel::onNodeSelected(int index)
{
  (void)index;
  refreshLogDisplay();
}

void TriggerGeminiPanel::refreshLogDisplay()
{
  log_display_->clear();
  QString selected_node = node_selector_->currentText();
  
  for (const auto& log : all_logs_) {
    if (log.name == selected_node) {
      bool show = false;
      if (log.level == rcl_interfaces::msg::Log::INFO && info_cb_->isChecked()) show = true;
      if (log.level == rcl_interfaces::msg::Log::WARN && warn_cb_->isChecked()) show = true;
      if ((log.level == rcl_interfaces::msg::Log::ERROR || log.level == rcl_interfaces::msg::Log::FATAL) && error_cb_->isChecked()) show = true;
      
      if (show) {
        appendLog(log.level, log.msg);
      }
    }
  }
}

void TriggerGeminiPanel::appendLog(int level, const QString & msg)
{
  QString prefix;
  QString color;

  switch (level) {
    case rcl_interfaces::msg::Log::INFO:
      prefix = "[INFO]";
      color = "black";
      break;
    case rcl_interfaces::msg::Log::WARN:
      prefix = "[WARN]";
      color = "orange";
      break;
    case rcl_interfaces::msg::Log::ERROR:
      prefix = "[ERROR]";
      color = "red";
      break;
    case rcl_interfaces::msg::Log::FATAL:
      prefix = "[FATAL]";
      color = "red";
      break;
    default:
      return; 
  }

  QScrollBar *vbar = log_display_->verticalScrollBar();
  // Check if we are at the bottom (with a small epsilon for safety)
  bool at_bottom = (vbar->value() >= vbar->maximum() - 10);

  QString html = QString("<font color=\"%1\">%2 %3</font>").arg(color, prefix, msg.toHtmlEscaped());
  log_display_->append(html);

  if (at_bottom) {
    vbar->setValue(vbar->maximum());
  }
}

void TriggerGeminiPanel::updateTargetClass()
{
  target_class_str_ = target_class_editor_->text();
}

void TriggerGeminiPanel::onButtonClick()
{
  // 1. Publish the Target Class
  if (publisher_) {
    auto msg = std_msgs::msg::String();
    msg.data = target_class_str_.toStdString();
    publisher_->publish(msg);
    RCLCPP_INFO(node_->get_logger(), "Published target class: %s", msg.data.c_str());
  }

  // 2. Wait 100ms
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // 3. Call Service
  if (!client_) return;

  if (!client_->wait_for_service(std::chrono::seconds(1))) {
    RCLCPP_WARN(node_->get_logger(), "Service /trigger_gemini_pick not available");
    return;
  }

  auto request = std::make_shared<std_srvs::srv::Trigger::Request>();
  auto future = client_->async_send_request(request);
  RCLCPP_INFO(node_->get_logger(), "Sent request to /trigger_gemini_pick");
}

void TriggerGeminiPanel::load(const rviz_common::Config & config)
{
  rviz_common::Panel::load(config);
  QString str;
  if (config.mapGetString("TargetClass", &str)) {
    target_class_editor_->setText(str);
    updateTargetClass();
  }
}

void TriggerGeminiPanel::save(rviz_common::Config config) const
{
  rviz_common::Panel::save(config);
  config.mapSetValue("TargetClass", target_class_str_);
}

} // namespace qb_arm_rviz_plugins

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(qb_arm_rviz_plugins::TriggerGeminiPanel, rviz_common::Panel)
