#include "qb_arm_rviz_plugins/trigger_gemini_panel.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <rviz_common/display_context.hpp>

namespace qb_arm_rviz_plugins
{

TriggerGeminiPanel::TriggerGeminiPanel(QWidget * parent)
: rviz_common::Panel(parent), service_name_("/trigger_gemini_pick")
{
  QVBoxLayout* layout = new QVBoxLayout;
  
  QHBoxLayout* service_layout = new QHBoxLayout;
  service_layout->addWidget(new QLabel("Service:"));
  service_name_editor_ = new QLineEdit(service_name_);
  service_layout->addWidget(service_name_editor_);
  layout->addLayout(service_layout);

  button_ = new QPushButton("Trigger");
  layout->addWidget(button_);
  
  setLayout(layout);

  connect(button_, SIGNAL(clicked()), this, SLOT(onButtonClick()));
  connect(service_name_editor_, SIGNAL(editingFinished()), this, SLOT(updateService()));
}

TriggerGeminiPanel::~TriggerGeminiPanel()
{
}

void TriggerGeminiPanel::onInitialize()
{
  node_ = getDisplayContext()->getRosNodeAbstraction().lock()->get_raw_node();
  updateService();
}

void TriggerGeminiPanel::updateService()
{
  QString new_service_name = service_name_editor_->text();
  if (new_service_name != service_name_ || !client_) {
    service_name_ = new_service_name;
    if (node_) {
      client_ = node_->create_client<std_srvs::srv::Trigger>(service_name_.toStdString());
    }
  }
}

void TriggerGeminiPanel::onButtonClick()
{
  if (!client_) return;

  if (!client_->wait_for_service(std::chrono::seconds(1))) {
    RCLCPP_WARN(node_->get_logger(), "Service %s not available", service_name_.toStdString().c_str());
    return;
  }

  auto request = std::make_shared<std_srvs::srv::Trigger::Request>();
  auto future = client_->async_send_request(request);
  RCLCPP_INFO(node_->get_logger(), "Sent request to %s", service_name_.toStdString().c_str());
}

void TriggerGeminiPanel::load(const rviz_common::Config & config)
{
  rviz_common::Panel::load(config);
  QString str;
  if (config.mapGetString("Service", &str)) {
    service_name_editor_->setText(str);
    updateService();
  }
}

void TriggerGeminiPanel::save(rviz_common::Config config) const
{
  rviz_common::Panel::save(config);
  config.mapSetValue("Service", service_name_);
}

} // namespace qb_arm_rviz_plugins

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(qb_arm_rviz_plugins::TriggerGeminiPanel, rviz_common::Panel)