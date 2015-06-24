
> Just a tool for PHP corders.

#Auto ID

A Drupal module for generating datetime-based serial numbers, for example `SALES201502280001`.




##Usage

Install the module, Then you can find configuration link in `admin/config`.

You can define new schema in the config page.

At last , you can use `date_serial::new_id($module_name, $prefix)` in your **PHP code** to generate a new serial number.


---

> 模块流请忽略本模块 

#流水号

用于生成流水号（例如`SALES201502280001`）的一个简单模块。

##用法

安装模块，在配置页面能看到该模块的配置入口。

可以在配置页面生成新的ID样式。

接下来就可以在**代码中**使用`date_serial::new_id($module_name, $prefix)`的形式来生成新的流水号了
