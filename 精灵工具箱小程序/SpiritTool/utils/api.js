//中国移动 OneENT 下发命令
function OneENT_SendCmd(device_id, api_key, cmd) {
	//小程序请求 
	return new Promise((resolve, reject) => {
		uni.request({
			url: "https://api.heclouds.com/cmds?device_id=" + device_id,
			method: "POST",
			header: {
				"api-key": api_key
			},
			data: cmd,
			success: (res) => { 
				console.log(res.errMsg);
				if (res.errMsg == "request:ok") {
					resolve(res.errMsg); // 千万别忘写！！！
				}
			},
			fail(err) {
				resolve("err"); // 千万别忘写！！！
			}
		});
	})
}
//中国移动 OneENT 查询设备
function OneENT_GetDevice(device_id, api_key) {
	//小程序请求 下发命令
	return new Promise((resolve, reject) => {
		uni.request({
			url: "https://api.heclouds.com/devices/" + device_id,
			method: "GET",
			header: {
				"api-key": api_key
			},
			success: (res) => { 
				if (res.errMsg == "request:ok") {
					resolve(res.data.data); // 千万别忘写！！！
				}
			},
			fail(err) {
				resolve("err"); // 千万别忘写！！！
			}
		});
	})
}
module.exports = {
	OneENT_SendCmd: OneENT_SendCmd,
	OneENT_GetDevice :OneENT_GetDevice 
}