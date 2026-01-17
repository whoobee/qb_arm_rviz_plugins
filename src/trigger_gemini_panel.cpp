#include "qb_arm_rviz_plugins/trigger_gemini_panel.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <rviz_common/display_context.hpp>
#include <thread>
#include <chrono>

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
  
  setLayout(layout);

  connect(button_, SIGNAL(clicked()), this, SLOT(onButtonClick()));
  connect(target_class_editor_, SIGNAL(editingFinished()), this, SLOT(updateTargetClass()));
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
