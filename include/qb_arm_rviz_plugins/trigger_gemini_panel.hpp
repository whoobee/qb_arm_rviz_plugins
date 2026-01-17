#ifndef QB_ARM_RVIZ_PLUGINS__TRIGGER_GEMINI_PANEL_HPP_
#define QB_ARM_RVIZ_PLUGINS__TRIGGER_GEMINI_PANEL_HPP_

#include <rviz_common/panel.hpp>
#include <rclcpp/rclcpp.hpp>
#include <std_srvs/srv/trigger.hpp>
#include <std_msgs/msg/string.hpp>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>

namespace qb_arm_rviz_plugins
{

class TriggerGeminiPanel : public rviz_common::Panel
{
  Q_OBJECT
public:
  explicit TriggerGeminiPanel(QWidget * parent = 0);
  virtual ~TriggerGeminiPanel();

  virtual void onInitialize() override;
  virtual void load(const rviz_common::Config & config) override;
  virtual void save(rviz_common::Config config) const override;

protected Q_SLOTS:
  void onButtonClick();
  void updateTargetClass();

protected:
  QPushButton* button_;
  QLineEdit* target_class_editor_;
  QString target_class_str_;

  rclcpp::Client<std_srvs::srv::Trigger>::SharedPtr client_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
  rclcpp::Node::SharedPtr node_;
};

} // namespace qb_arm_rviz_plugins

#endif // QB_ARM_RVIZ_PLUGINS__TRIGGER_GEMINI_PANEL_HPP_
