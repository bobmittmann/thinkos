var stdout_flush_tmo = [500, 500, 500, 500, 500];

function simlib_stdout_flush(port, pre) {
	$.ajax({
		type: 'GET',
		url: 'get_stdout.cgi',
		data: { 
			port: port,
		},
		dataType: 'text',
		timeout: 100,
		context: $('body'),
		success: function(data) {
			if (data) {
				pre.append(data);
				pre.scrollTop(pre.prop("scrollHeight"));
			}
		},
		error: function(xhr, type) {
		},
		complete: function(xhr, status) {
		}
	});
};

function simlib_microjs_ctl(port, ctl)
{
	$.ajax({
		type: 'GET',
		url: 'sim_microjs_ctl.cgi',
		data: { 
			port: port,
			ctl: ctl
		},
		dataType: 'json',			
		timeout: 250,
		context: $('body'),
		success: function(data) {
		},
		error: function(xhr, type) { 
		}
	});
}

function simlib_dev_enable_set(port, dev_type, dev_addr, enable)
{
	$.ajax({
		type: 'GET',
		url: 'dev_enable_set.cgi',
		data: { 
			port: port,
			dev_type: dev_type,
			dev_addr, dev_addr,
			enable: enable
		},
		dataType: 'json',			
		timeout: 125,
		success: function(data) {
		},
		error: function(xhr, type) { 
			$('#msg').html('Error +xhr +type');
		},
		complete: function(xhr, status){
			$('#msg').html('');
			$('body').css("cursor", "default");
			$('.button').prop('disabled', false);
		}
	});
}

function simlib_dev_alarm_set(port, dev_type, dev_addr, level)
{
	$.ajax({
		type: 'GET',
		url: 'dev_alarm_set.cgi',
		data: { 
			port: port,
			dev_type: dev_type,
			dev_addr, dev_addr,
			level: level
		},
		dataType: 'json',			
		timeout: 125,
		success: function(data) {
		},
		error: function(xhr, type) { 
		}
	});
}

function simlib_dev_trouble_set(port, dev_type, dev_addr, level)
{
	$.ajax({
		type: 'GET',
		url: 'dev_trouble_set.cgi',
		data: { 
			port: port,
			dev_type: dev_type,
			dev_addr, dev_addr,
			level: level
		},
		dataType: 'json',			
		timeout: 125,
		success: function(data) {
		},
		error: function(xhr, type) { 
		}
	});
}


function db_status(port, tr) {
	$('.button').prop('disabled', true);
	
	$.ajax({
		type: 'GET',
		url: 'db_getinfo.cgi',
		data: { 
			port: port
		},
		dataType: 'json',			
		timeout: 500,
		context: $('body'),
		success: function(data) {	
			if (data.hasOwnProperty('desc')) {
		     var desc = data.desc;
			 if (desc == '')
			 desc = '&lt;none&gt;'
			tr.cells[1].innerHTML = 'DB- ' + data.version.major + '.' + 
				data.version.minor + '.' +
				data.version.build + desc;
		}
		else {
			tr.cells[1].innerHTML = '<span style="color: red;">'+'<i>' + data.error + '</i>'+'</span>';
		}},

	   error: function(xhr, type) { 
		tr.cells[1].innerHTML= (' Ajax error: ' + type  );
		},
		complete: function(xhr, status){
			$('.button').prop('disabled', false);
		}

	});
}

function port_status(port, tr){
	$.ajax({
		type: 'GET',
		url: 'get_status.cgi',
		data: { 
			port: port
		},
		dataType: 'json',			
		timeout: 500,
		context: $('body'),
		success: function(data) {	
			tr.cells[2].innerHTML = data.state;
			},
			error: function(xhr, type) { 
				stdout_flush_tmo[port] = 5000;
			}
		});
	}



function simlib_status_get(port, tr) {	
	$.ajax({
		type: 'GET',
		url: 'get_status.cgi',
		data: { 
			port: port
		},
		dataType: 'json',			
		timeout: 250,
		context: $('body'),
		success: function(data) {
			
			tr.cells[1].innerHTML = data.state;
			if (data.hasOwnProperty('kernel')) {
				tr.cells[2].innerHTML = 'ThinkOS-' + data.kernel.version.major + '.' + 
					data.kernel.version.minor + '.' +
					data.kernel.version.build;
				tr.cells[3].innerHTML = data.kernel.ticks;
			} else {
				tr.cells[2].innerHTML = '?.?.?';
				tr.cells[3].innerHTML = '?';
			}
			if (data.hasOwnProperty('app')) {
				tr.cells[4].innerHTML = 'DevSim-' + data.app.version.major + '.' + 
					data.app.version.minor + '.' +
					data.app.version.build;
				tr.cells[5].innerHTML = data.app.uptime;						
			} else {
				tr.cells[4].innerHTML = '?.?.?';
				tr.cells[5].innerHTML = '?';
			}
		},
		error: function(xhr, type) { 
			stdout_flush_tmo[port] = 5000;
		}
	});
}

function simlib_js_post(port, script) {		
	$.ajax({
 		type: 'POST',
		url: 'rpc_js.cgi?port=' + port,
 		data: script,
		dataType: 'text',
		timeout: 250,
		context: $('body'),
		contentType: 'json',
		success: function(data) {
   		},
		error: function(xhr, type){
			stdout_flush_tmo[port] = 5000;
		},
		complete: function() {
		}
	});
};

function alert_show(__box, __msg) {		
	var html = '<img style="float:none;vertical-align:middle"' +
		'src="/img/alert.png" alt="Alert! "/>&nbsp;' + __msg;
	__box.innerHTML = html;
}

function alert_hide(__box) {
	__box.innerHTML = "";
}

function simlib_edit_file_read(port, fname, editor) {
	var stat= $('#status');
	editor.text('');	
	var html= '<span style="color:red">'+"Reading file, please wait..."+'</span>';
	stat.html(html);
	$('.button').prop('disabled', true);
	$.ajax({
		type: 'GET',
		url: 'file_read.cgi',
		data: { 
			fname: fname,
			port: port,
		},
		dataType: 'text',
		timeout: 5000,
/* async: false, */
		context: $('body'),
		success: function(data) {
			editor.text(data);
			stat.html('Read Successfully');
		},
		error: function(xhr, type) {
			stat.html('Ajax error');
		},
		complete: function(xhr, status) {
			$('.button').prop('disabled', false);
		}
	});
}

function simlib_edit_file_upload(port, editor, form, __box) {
	editor.text('');
	var stat = $('#status');
	var html= '<span style="color:red">'+"Uploading file, please wait..."+'</span>';
	stat.html(html);
	$('.button').prop('disabled', true);
	
	$.ajax({
		url: form.action + '?port=' + port,
		type: 'POST',
		data: new FormData(form),
		processData: false,
		contentType: false,
		// async: false,
		success: function(data) {				
			editor.text(data);
			var html= '<span style="color:green">'+"Uploaded Successfully"+'</span>';
			stat.html(html);
		},
		error: function(xhr, type) {
			var html= '<span style="color:red">'+xhr +type +'</span>';
			stat.html(html);
		},
		complete: function(xhr, status) {
			$('.button').prop('disabled', false);
		}
	});
}

function simlib_edit_file_write(port, fname, editor, __box) {	
	var text = editor.val();
	var stat = $('#status');
	var html= '<span style="color:red">'+"Writing file, please wait..."+'</span>';
	stat.html(html);
	
	$('.button').prop('disabled', true);	

	$.ajax({
 		type: 'POST',
		url: 'file_write.cgi?port=' + port + '&fname=' + fname,
 		data: text,
		dataType: 'text',
		timeout: 5000,
/* async: false, */
		context: $('body'),
		contentType: 'text/plain',
		success: function(data) {
			var html= '<span style="color:green">'+"Written Successfully"+'</span>';
			stat.html(html);
   		},
		error: function(xhr, type){
			stdout_flush_tmo[port] = 5000;
		},
		complete: function() {
			$('.button').prop('disabled', false);	
		}
	});
};

function simlib_file_erase(port, fname, success)
{
	$.ajax({
		type: 'GET',
		url: 'sim_file_erase.cgi',
		data: { 
			port: port,
			fname: fname
		},
		dataType: 'json',			
		timeout: 500,
		success: function(data) {
			if (!(typeof success === 'undefined')) {			
				success();
			}
		},
		error: function(xhr, type) { 
		}
	});
}

const SIMRPC_SIMCTL_STOP = 0;
const SIMRPC_SIMCTL_RESUME = 1;

function simlib_sim_stop(port, on_success)
{
	$.ajax({
		type: 'GET',
		url: 'sim_ctl.cgi',
		data: { 
			port: port,
			ctl: SIMRPC_SIMCTL_STOP
		},
		dataType: 'json',			
		timeout: 250,
		context: $('body'),
		success: function(data) {
			console.log('sim_ctl.cgi: ' + data);
			if (!(typeof on_success === 'undefined')) {			
				on_success();
			}
		},
		error: function(xhr, type) { 
			console.log('sim_ctl.cgi failed: ' + type);
		}
	});
}

function simlib_sim_resume(port)
{
	$.ajax({
		type: 'GET',
		url: 'sim_ctl.cgi',
		data: { 
			port: port,
			ctl: SIMRPC_SIMCTL_RESUME
		},
		dataType: 'json',			
		timeout: 200,
		success: function(data) {
		},
		error: function(xhr, type) { 
		}
	});
}
