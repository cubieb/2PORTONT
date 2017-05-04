<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!--系统默认模板-->
<html>
<head>
<title>QoS 添加</title>
<META http-equiv=pragma content=no-cache>
<META http-equiv=cache-control content="no-cache, must-revalidate">
<META http-equiv=content-type content="text/html; charset=gbk">
<META http-equiv=content-script-type content=text/javascript>
<!--系统公共css-->
<style type=text/css>
@import url(/style/default.css);
</style>
<!--系统公共脚本-->
<script language="javascript" src="common.js"></script>
<script language="javascript" type="text/javascript">
var policy = 1;
var waniflst = new it_nr("waniflist");
<% initQueuePolicy(); %>
<% ifWanList(); %>
function on_init(){
	with(document.forms[0]){
		if(policy != 0 && policy !=1)
			policy = 0;
		queuepolicy[policy].checked = true;
		
		for(var i in waniflst){
			
			if(i == "name"||i=="undefined" ||(typeof waniflst[i] != "string" && typeof waniflst[i] != "number")) continue;
			queueinflist.options.add(new Option(waniflst[i],i));
		}
	}
}
function on_save() {
	with(document.forms[0]) {
		var sbmtstr = "";
		if(queuepolicy[0].checked==true)
			sbmtstr = "policy=0";
		else
			sbmtstr = "policy=1";
		if(queuestatus.value == -1) {
			alert("请选择队列配置状态值！");
			return;
		} else {
			sbmtstr = sbmtstr+"&queuestatus="+queuestatus.value;
		}
		
		if(queueinflist.value==-1) {
			alert("请选择队列值！");
			return;
		} else {
			sbmtstr = sbmtstr+"&queueinf="+queueinflist.value;
		}
		
		if(queueprio.value == 0) {
			alert("请选择队列优先级！");
			return;
		} else {
			sbmtstr = sbmtstr+"&queueprio="+queueprio.value;
		}
		lst.value = sbmtstr;
		submit();
	}	
}
</script>
</head>
<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init();">
<blockquote>
	<DIV align="left" style="padding-left:20px; padding-top:5px;">
		<form id="form" action="/boaform/admin/formQosQueueEdit" method="post">
		  <p><strong>QoS 队列配置</strong></p>
		  <p>本页面主要配置具体每个网络接口的QoS队列条目。每个网络接口启用QoS后最多可设置4个队列。每个队列可以指定一个优先级。QoS类型规则根据所配置的队列条目正确放置上行数据包到相应队列。注意：数值越小，优先级越高。如果选择的策略为WRR，则优先级为1，2，3，4的这四个队列的权重比是4:3:2:1。例如，对某一网络接口只配置了优先级为1和4的两个队列，则它们的权重比为4:1。策略选择项标识当前应用策略。点击”保存/应用“   保存并激活设置。     </p>
		  <table class="flat" id="queueadd_tbl" border="0" cellpadding="0" cellspacing="1">
		  	<tr>
		  		<td>策略选择:</td>
				<td><input type="radio"  name="queuepolicy" value="prio">绝对优先级策略</td>	
				<td><input type="radio"  name="queuepolicy" value="wrr">WRR策略</td>	
			</tr>
			<tr>
				<td>队列配置状态：</td>
				<td><select name="queuestatus" style="width:200px">
						<option value="-1">&nbsp;</option>
						<option value="0">禁用</option>
						<option value="1">启用</option>
					</select>
				</td>
			</tr>
			<tr>
				<td>队列：</td>
				<td><select name="queueinflist" style="width:200px"></select>
				</td>
			</tr>
			<tr>
				<td>队列优先级：</td>
				<td><select name="queueprio" style="width:200px">
						<option value="0">&nbsp;</option>
						<option value="1">1</option>
						<option value="2">2</option>
						<option value="3">3</option>
						<option value="4">4</option>
					</select>
				</td>
			</tr>
		  </table>
		  <input type="button" class="button" value="保存" onClick="on_save();">
		  <input type="hidden" id="lst" name="lst" value="">
		  <input type="hidden" name="submit-url" value="/net_qos_queue.asp">
		</form>
	</DIV>
</blockquote>
</body>
<%addHttpNoCache();%>
</html>
