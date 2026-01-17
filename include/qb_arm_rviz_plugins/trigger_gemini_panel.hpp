#ifndef QB_ARM_RVIZ_PLUGINS__TRIGGER_GEMINI_PANEL_HPP_
#define QB_ARM_RVIZ_PLUGINS__TRIGGER_GEMINI_PANEL_HPP_

#include <rviz_common/panel.hpp>
#include <rclcpp/rclcpp.hpp>
#include <std_srvs/srv/trigger.hpp>
#include <std_msgs/msg/string.hpp>
#include <rcl_interfaces/msg/log.hpp>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QTimer>
#include <deque>

namespace qb_arm_rviz_plugins
{

struct LogMessage {
  int level;
  QString name;
  QString msg;
};

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
  void appendLog(int level, const QString & msg);
  void updateNodeList();
  void onNodeSelected(int index);
  void onLevelFilterChanged(int state);

protected:
  void refreshLogDisplay();

  QPushButton* button_;
  QLineEdit* target_class_editor_;
  QComboBox* node_selector_;
  QCheckBox* info_cb_;
  QCheckBox* warn_cb_;
  QCheckBox* error_cb_;
  QTextEdit* log_display_;
  QString target_class_str_;

  QTimer* discovery_timer_;
  std::deque<LogMessage> all_logs_;
  const size_t max_logs_ = 500;

  rclcpp::Client<std_srvs::srv::Trigger>::SharedPtr client_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
  rclcpp::Subscription<rcl_interfaces::msg::Log>::SharedPtr log_sub_;
  rclcpp::Node::SharedPtr node_;
};

} // namespace qb_arm_rviz_plugins

#endif // QB_ARM_RVIZ_PLUGINS__TRIGGER_GEMINI_PANEL_HPP_
