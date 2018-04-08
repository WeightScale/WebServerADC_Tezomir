// Page.h

#ifndef _PAGE_h
#define _PAGE_h

const char index_html[] PROGMEM = R"(<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1, maximum-scale=1.0, user-scalable=no'/><meta http-equiv='Cache-Control' content='no-cache, no-store, must-revalidate'/><meta http-equiv='Pragma' content='no-cache'/>
<title>WEB SCALES</title><link rel='stylesheet' type='text/css' href='global.css'><link rel='shortcut icon' href='favicon.png' type='image/png'><style>#w_style{background:#fff;font-size:80px;font-family:Arial,sans-serif;color:#618ad2;margin-left:auto;margin-right:auto;}table {width:100%;}input{font-size:20px;text-align:center;}</style>
<script>var w;function ScalesSocket(h, p, fm, fe){var host=h;var protocol=p;var timerWeight;var timerSocket;var ws;var startWeightTimeout=function(){clearTimeout(timerWeight);timerWeight=setTimeout(function(){fe();},5000);};
this.getWeight=function(){ws.send('/wt');startWeightTimeout();};this.openSocket=function(){ws=new WebSocket(host,protocol);ws.onopen=this.getWeight;ws.onclose=function(e){clearTimeout(timerWeight);starSocketTimeout();fe();};ws.onerror=function(e){fe();};ws.onmessage=function(e){fm(JSON.parse(e.data));}};var starSocketTimeout=function(){clearTimeout(timerSocket);timerSocket=setTimeout(function(){this.prototype.openSocket();},5000);}}
function go(){document.getElementById('weight').innerHTML='---';document.getElementById('charge_id').innerHTML='--%';document.getElementById('buttonZero').style.visibility='visible';}
function setZero(){document.getElementById('buttonZero').style.visibility='hidden';var request=new XMLHttpRequest();document.getElementById('weight').innerHTML='...';request.onreadystatechange=function(){if(this.readyState===4 && this.status===204){document.getElementById('buttonZero').style.visibility='visible';w.getWeight();}};request.open('GET','/tp',true);request.timeout=5000;request.ontimeout=function(){go();};request.onerror=function(){go();};request.send(null);}
window.onload=function(){onBodyLoad();};function onBodyLoad(){w = new ScalesSocket('ws://'+document.location.host+'/ws',['arduino'],function(e){try{document.getElementById('weight').innerHTML=e.w;document.getElementById('charge_id').innerHTML=e.c+'%';if(e.s){document.getElementById('w_style').setAttribute('style','background: #fff;');}else{document.getElementById('w_style').setAttribute('style','background: #C4C4C4;');}}catch(e){go();}finally{w.getWeight();}},function(){go();w.openSocket();});	w.openSocket();}
</script></head><body><div align='center'><table><tr><td ><img src='logo.png' style='height: 42px;' /></td><td align='right'><h3 id='brand_name'>tenzomir.com.ua</h3></td></tr></table><p hidden='hidden' id='dnt'></p></div><hr><div align='right' id='w_style'><b id='weight'>---</b></div><hr><table><tr><td style='width:1%; white-space: nowrap'><img src='battery.png' alt='B'/></td><td><h3 id='charge_id' style='display: inline'>--%</h3></td><td align='right'><b><a href='javascript:setZero()' id='buttonZero' class='btn btn--s btn--blue'>&gt;0&lt;</a></b></td></tr></table><hr><table><tr><td><a href='/events.html' class='btn btn--m btn--blue'>события</a><br></td></tr><tr><td><br/><a href='/settings.html'>настройки</a><br></td></tr></table><hr><footer align='center'>2018 © Powered by konst</footer></body></html>)";


//settings.html
	const char settings_html[] PROGMEM = R"(<!DOCTYPE html><html lang="en"><head><meta charset="UTF-8"><meta name='viewport' content='width=device-width, initial-scale=1, maximum-scale=1.0, user-scalable=no'/><meta http-equiv="Cache-Control" content="no-cache, no-store, must-revalidate"/><meta http-equiv="Pragma" content="no-cache"/><title>Настройки</title><link rel="stylesheet" type="text/css" href="global.css"><style>input:focus{background: #FA6;outline: none;}table{width:100%;}input{width:100%;text-align:right;font-size:18px;}input[type=submit]{width:auto;}</style><script>
	function setOnBlur( input){
		setTimeout(function(){
			if (document.activeElement===input){
				input.onblur=function(){
					if(input.value.length===0 || !CheckIP(input)){
						setTimeout(function(){input.focus()},0)
					}else
						input.onblur=null
				}
			}
		},0)
	}
	function CheckIP(input){
		ipParts=input.value.split(".");
		if(ipParts.length===4){
			for(i=0;i<4;i++){
				TheNum=parseInt(ipParts[i]);
				if(TheNum>=0 && TheNum <= 255){}
				else{break;}
			}
			if(i==4)
			return true;
		}
		return false;
	}
	function sendDateTime(){
		var formData=new FormData();
		var date=new Date();
		var d=date.toLocaleDateString();
		d+="-"+date.toLocaleTimeString();
		formData.append('data',d.replace(/[^\x20-\x7E]+/g,''));
		var request=new XMLHttpRequest();
		request.onreadystatechange=function(){
			if (this.readyState===4 && this.status===200){
				if (this.responseText !== null){document.getElementById('id_date').innerHTML="<div>Обновлено<br/>"+this.responseText+"</div>";}
			}
		};
		request.open("POST","settings.html?"+new Date().getTime(),true);
		request.send(formData);
	}	
	function GetValue() {
		var http_request=new XMLHttpRequest();
		http_request.overrideMimeType('application/json');
		http_request.onreadystatechange=function(){
			if (this.readyState===4){
				if(this.status===200){
					var json=JSON.parse(this.responseText);
					for (entry in json) {
						try {document.getElementById(entry).innerHTML = json[entry];}catch (e){}
						}
					}
				}
			};
			http_request.open("GET","/sv",true);
			http_request.send(null);
		}
		function GetSettings() {
			var http_request=new XMLHttpRequest();
			http_request.overrideMimeType('application/json');
			http_request.onreadystatechange=function(){
				if (this.readyState===4){
					if(this.status===200){
						try{
							var json=JSON.parse(http_request.responseText);
							var scale=json.scale;
							for (entry in scale) {
								try {if(document.getElementById(entry).type==='checkbox'){document.getElementById(entry).checked=scale[entry];enableAuthFields(document.getElementById(entry));}else document.getElementById(entry).value=scale[entry];}catch (e){}}
								var server=json.server;
								for (entry in server){
									try{document.getElementById(entry).value=server[entry];}catch (e){}}}catch(e){alert("ОШИБКА "+e.toString());}
								}else{alert("ДАННЫЕ НАСТРОЕК НЕ НАЙДЕНЫ!!!");}
							document.body.style.visibility='visible';
							GetValue();
						}
					};
					http_request.open('GET','/settings.json', true);
					http_request.send(null);
				}
				window.onload=function(){GetSettings();};
				function openSDB(){
					var url='https://'+document.getElementById('id_host').value+'/scale.php?code='+document.getElementById('id_pin').value;
					var win=window.open(url,'_blank');
					win.focus();
				}
				function enableAuthFields(check){if(check.checked){document.getElementById('id_table_net').style.display='none';}else{document.getElementById('id_table_net').style.display='';}}
				function submitFormNet(f){
					var form=document.getElementById(f);
					var formData=new FormData(form);
					var request=new XMLHttpRequest();
					request.onreadystatechange=function(){
						if (this.readyState===4){if (this.status===200){var rec=confirm('Пересоеденится с новыми настройками');if(rec){this.onreadystatechange=null;this.open('GET','/rc',true);this.send(null);}}else if(this.status===400){alert('Ошибка при сохранении настроек')}}};
					request.open('POST','/settings.html',true);
					request.send(formData);
				}
				function submitFormServer() {
					var form=document.getElementById('form_server_id');
					var formData=new FormData(form);
					var request=new XMLHttpRequest();
					request.onreadystatechange=function(){if(this.readyState===4){if(this.status===200){alert('OK');}else if(this.status===400){alert('ERROR');}}};
					request.open('POST','/settings.html',true);
					request.send(formData);
				}
				</script>
				</head>
				<body style='visibility: hidden'>
				<a href='/'  class='btn btn--s btn--blue'>&lt;</a>&nbsp;&nbsp;<strong>Настройки</strong>
				<hr>
				<fieldset><details><summary>Конфигурация сети</summary>
				<br>
				<h5 align='left'><b>Точка доступа WiFi роутера</b></h5>
				<form id='form_id' action='javascript:submitFormNet("form_id")'>
				Получать IP:
				<input type='checkbox' id='id_auto' name='auto' onclick='enableAuthFields(this);'>
				<div id='id_ip'></div>
				<table id='id_table_net' ><tr><td>IP:</td><td><input id='id_lan_ip' type='text' name='lan_ip'  onfocus='setOnBlur(this)'></td></tr><tr><td>ШЛЮЗ:</td><td><input id='id_gateway' type='text' name='gateway'  onfocus='setOnBlur(this)'></td></tr><tr><td>МАСКА:</td><td><input id='id_subnet' type='text' name='subnet'  onfocus='setOnBlur(this)'></td></tr></table>
				<table><tr><td>СЕТЬ:</td><td><input id='id_ssid' name='ssid' required placeholder='имя сети'></td></tr><tr><td>КЛЮЧ:</td><td><input id='id_key' type='password'  name='key' placeholder='пароль'></td></tr><tr><td></td><td><input type='submit' value='СОХРАНИТЬ'/></td></tr></table>
				</form>
				</details>
				</fieldset><br/>
				<fieldset style='width: auto'><details><summary>Общии настройки</summary>
				<br>
				<form action='javascript:sendDateTime()'><h5 align='left'><b>Установка дата время</b></h5>
				<table><tr><td><h5 id='id_date'>дата время</h5></td><td><input type='submit' name='data' value='УСТАНОВИТЬ'/></td></tr></table>
				</form>
				<hr>
				<form id='form_server_id' action='javascript:submitFormServer()'>
				<h5>Настройки база данных интернет</h5>
				<table><tr><td>СЕРВЕР:</td><td ><input id='id_host' name='host' placeholder='сервер'></td></tr><tr><td>ПИН:</td><td><input id='id_pin' name='pin' placeholder='пин весов'></td></tr><tr><td><a href='javascript:openSDB();'>открыть</a></td><td><input id='id_submit_code' type='submit' value='СОХРАНИТЬ'/></td></tr></table>
				</form>
				<hr>
				<form method='POST'><h5>Доступ к настройкам</h5>
				<table><tr><td>ИМЯ:</td><td><input id='id_n_admin' name='n_admin' placeholder='имя админ'></td></tr><tr><td>ПАРОЛЬ:</td><td><input type='password' id='id_p_admin' name='p_admin' placeholder='пароль админ'></td></tr><tr><td></td><td><input type='submit' value='СОХРАНИТЬ'/></td></tr></table>
				</form>
				</details>
				</fieldset><br/>
				<fieldset><details><summary>Информация</summary>
				<br><table><tr><td><h5>Имя хоста:</h5></td><td align='right'><h5 id='id_local_host'></h5></td></tr></table><hr>
				<h5 align='left'><b>Точка доступа весов</b></h5>
				<table><tr><td id='id_ap_ssid'></td><td align='right' id='id_ap_ip'></td></tr></table><hr>
				<table><tr><td>Пломба:</td><td align='right'><div id='sl_id'></div></td></tr></table><hr>
				<a href='/calibr.html'>калибровка</a>
				</details>
				</fieldset>
				<hr>
				<footer align='center'>2018 © Powered by konst</footer>
				</body>
				</html>)";

//calibr.html
const char calibr_html[] PROGMEM = R"(
<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1, maximum-scale=1.0, user-scalable=no'/><meta http-equiv='Cache-Control' content='no-cache, no-store, must-revalidate'/><meta http-equiv='Pragma' content='no-cache'/><title>Калибровка</title><link rel='shortcut icon' href='favicon.png' type='image/png'><link rel='stylesheet' type='text/css' href='global.css'>
<style>#wt_id{display:inline;background: #fff;font-size:28px;font-family:Arial,sans-serif;color:#618ad2;margin-left:auto;margin-right:auto;}table{width:100%;}input{width:100%;text-align:right;font-size:18px;height:auto;}input[type='submit']{width:auto;}select{width:100%;text-align-last:right;font-size:18px;height:auto;border:1px solid #ccc;}</style><script>
var weight;var w;
function ScalesSocket(h,p,fm,fe){
	var host=h;var protocol=p;var timerWeight;var timerSocket;var ws;
	var startWeightTimeout=function(){clearTimeout(timerWeight);timerWeight=setTimeout(function(){fe();},5000);};
	this.getWeight=function(){ws.send('/wt');startWeightTimeout();};
	this.openSocket=function(){
		ws=new WebSocket(host,protocol);
		ws.onopen=this.getWeight;
		ws.onclose=function(e){clearTimeout(timerWeight);starSocketTimeout();fe();};
		ws.onerror=function(e){fe();};
		ws.onmessage=function(e){fm(JSON.parse(e.data));}
	};
	var starSocketTimeout=function(){clearTimeout(timerSocket);timerSocket=setTimeout(function(){this.prototype.openSocket();},5000);}
}
function go(){document.getElementById('wt_id').innerHTML='---';}
function setMax(){var form=document.getElementById('form_c_id');var formData=new FormData(form);formData.append('update',true);var request=new XMLHttpRequest();request.onreadystatechange=function(){if (this.readyState===4 && this.status===200){if (this.responseText !== null){if(document.getElementById("form_zero")===null){document.getElementById("id_bs").value='ОБНОВИТЬ';var f=document.createElement('fieldset');f.id='form_zero';f.innerHTML="<legend>Нулевой вес</legend><form  action='javascript:setZero()'><p>Перед установкой убедитесь что весы не нагружены.</p><input type='submit' value='УСТАНОВИТЬ НОЛЬ'/></form><br><div id='wt_id'>---</div>";document.body.appendChild(f);setupWeight();}}}};request.open('POST','calibr.html',true);request.send(formData);}
function setZero(){var req=new XMLHttpRequest();var formData=new FormData();formData.append('zero',true);formData.append('weightCal','0');req.onreadystatechange=function(){if(this.readyState===4 && this.status===200){if(this.responseText !== null){document.getElementById('form_zero').disabled=true;var f=document.createElement('fieldset');f.id='form_weight';
	f.innerHTML="<legend>Калиброваный вес</legend><form action='javascript:setWeight()'><p>Перед установкой весы нагружаются контрольным весом. Дать некоторое время для стабилизации.Значение вводится с точностью которое выбрано в пункте Точность измерения.</p><table><tr><td>ГИРЯ:</td><td><input id='gr_id' value='0' type='number' step='any' required placeholder='Калиброваная гиря'/></td></tr><tr><td>ГРУЗ:</td><td><input id='id_cal_wt' value='0' type='number' step='any' title='Введите значение веса установленого на весах'  max='100000' required placeholder='Калиброваный вес'/></td></tr><tr><td>ОШИБКА:</td><td><div id='dif_gr_id'></div></td></tr><tr><td><input type='submit' value='УСТАНОВИТЬ'/></td><td><a href='javascript:calculate();'>подобрать</a></td></tr></table></form>";
document.body.appendChild(f);}}};req.open('POST','calibr.html',true);req.send(formData);}
function setWeight(){var fd=new FormData();var w=parseFloat(document.getElementById('id_cal_wt').value) + parseFloat(document.getElementById('gr_id').value);fd.append('weightCal',w.toString());var request=new XMLHttpRequest();request.onreadystatechange=function(){if(this.readyState===4){if(this.status===200){if(this.responseText !== null){if(document.getElementById('form_seal')===null){var f=document.createElement('fieldset');f.id='form_seal';f.innerHTML = "<legend>Пломбировка</legend><form action='javascript:setSeal()'><left><p>Сохранение процесса калибровки. Данные калибровки сохраняются в память весов.</p><input type='submit' value='ОПЛОМБИРОВАТЬ'/></left></form>";document.body.appendChild(f);}}}else if(this.status === 400){alert(this.responseText);}}};request.open('POST','calibr.html',true);request.send(fd);}
function setSeal(){var req=new XMLHttpRequest();req.onreadystatechange=function(){if(this.readyState===4 && this.status===200){alert('Номер пломбы: '+this.responseText);window.location.replace('/');}};req.open('GET','/sl',true);req.send(null);}
function GetSettings(){var req=new XMLHttpRequest();req.overrideMimeType('application/json');req.onreadystatechange=function(){if(req.readyState===4){try{var json=JSON.parse(req.responseText);for(entry in json){try{if(document.getElementById(entry)!== null)document.getElementById(entry).value=json[entry];}catch(e){}}}catch(e){alert(e.toString());}document.body.style.visibility='visible';}};req.open('GET','/cdate.json',true);req.send(null);}
window.onload=function(){GetSettings();};
function saveValue(){var f=document.getElementById('form_c_id');var fd=new FormData(f);fd.append('update',true);var req=new XMLHttpRequest();req.onreadystatechange=function(){if(this.readyState===4 && this.status===200){if(this.responseText !== null){window.open('/','_self');}}};req.onerror = function(){alert('Ошибка');};req.open('POST','calibr.html',true);req.send(fd);};
function getWeight(){w=new ScalesSocket('ws://'+document.location.host+'/ws',['arduino'],handleWeight,function(){go();w.openSocket();});w.openSocket();}
function calculate(){var dif=weight-parseFloat(document.getElementById('id_cal_wt').value);dif=parseFloat(document.getElementById('gr_id').value)/dif;dif=parseFloat(document.getElementById('id_cal_wt').value)*dif;document.getElementById('id_cal_wt').value=dif.toFixed(document.getElementById('ac_id').value);setWeight();}
function handleWeight(d){weight=d.w;document.getElementById('wt_id').innerHTML=weight;try{document.getElementById('form_seal').disabled=(d.s===false);}catch(e){}if(d.s){document.getElementById('wt_id').setAttribute('style','background: #fff;');}else{document.getElementById('wt_id').setAttribute('style','background: #C4C4C4;');}try {var dif_gr=parseFloat(document.getElementById('id_cal_wt').value)+parseFloat(document.getElementById('gr_id').value);dif_gr -= weight;document.getElementById('dif_gr_id').innerHTML=dif_gr.toFixed(document.getElementById('ac_id').value);}catch (e){}w.getWeight();}
function setupWeight(){getWeight();}
</script>
</head>
<body style='visibility: hidden'>
<a href='/settings.html' class='btn btn--s btn--blue'>&lt;</a>&nbsp;&nbsp;<strong>Калибровка</strong>
<hr>
<fieldset id='form_max' style='visibility: visible'>
<legend>Общии настройки</legend>
<form id='form_c_id' action='javascript:setMax()'>
<table>
<tr><td>Точность измерения</td><td><select id='ac_id' name='weightAccuracy' title='Выбор точности с которым будет измерения'><option name='0' value='0'>0</option><option name='0.0' value='1'>0.0</option><option name='0.00' value='2'>0.00</option><option name='0.000' value='3'>0.000</option></select></td></tr>
<tr><td>Шаг измерения</td><td><select id='st_id' name='weightStep' title='Выбор шага измерения'><option name='шаг 1' value='1'>1</option><option name='шаг 2' value='2'>2</option><option name='шаг 5' value='5'>5</option><option name='шаг 10' value='10'>10</option><option name='шаг 20' value='20'>20</option><option name='шаг 50' value='50'>50</option></select></td></tr>
<tr><td>Кол-во измерений</td><td><select id='av_id' name='weightAverage' title='Выбор количества измерений АЦП'><option name='одно' value='1'>ОДИН</option><option name='два' value='2'>ДВА</option></select></td></tr>
<tr><td>Фильтр</td><td><select id='fl_id' name='weightFilter' title='Выбор значения фильтра. Чем меньше значение тем лутшее фильтрация но дольше измерение'><option name='5%' value='5'>5 %</option><option name='10%' value='10'>10 %</option><option name='20%' value='20'>20 %</option><option name='30%' value='30'>30 %</option>
<option name='40%' value='40'>40 %</option><option name='50%' value='50'>50 %</option><option name='60%' value='60'>60 %</option><option name='70%' value='70'>70 %</option><option name='80%' value='80'>80 %</option><option name='90%' value='90'>90 %</option><option name='100%' value='100'>100 %</option></select></td></tr>
<tr><td>НВП</td><td><input title='Введите значение максимального веса' type='number' min='1' max='100000' id='mw_id' name='weightMax' placeholder='Найбольший предел'></td></tr>
<tr><td><a href='javascript:saveValue();'>сохранить и выйти</a></td><td><input id='id_bs' type='submit' value='ДАЛЬШЕ >>'/></td></tr>
</table>
</form>
</fieldset>
<fieldset>
<details><summary>Авторизация для калибровки</summary>
<form method='POST'>
<table>
<tr><td>ИМЯ:</td><td><input id='us_id' name='user' placeholder='имя админ'></td></tr>
<tr><td>ПАРОЛЬ:</td><td><input type='password' id='ps_id' name='pass' placeholder='пароль админ'></td></tr>
<tr><td></td><td><input type='submit' value='СОХРАНИТЬ'/></td></tr>
</table>
</form>
</details>
</fieldset>
</body>
</html>)";

#endif







