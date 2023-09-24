<template>
	<view class="content">
		<view>
			<view class="uni-form-item uni-column">
				<view class="title">WiFiSSID</view>
				<input class="uni-input" :value="wifissid" @input="onssidInput"  />
			</view>
			<view class="uni-form-item uni-column">
				<view class="title">WiFiBSSID</view>
				<input class="uni-input" :value="wifiBssid" />
			</view>
			<view class="uni-form-item uni-column">
				<view class="title">PASS</view>
				<input class="uni-input" password @input="onKeyInput" placeholder-style="color:#F76260"
					placeholder="请输入无线网络密码" />
			</view>


			<view class="uni-form-item uni-column">
				<view class="title">AP-SSID</view>
				<input class="uni-input" :value="APSSID" />
			</view>
			<view class="uni-form-item uni-column">
				<view class="title">AP-PASS</view>
				<input class="uni-input" :value="APpassword" />
			</view>


		</view> 
		
		<button class="mini-btn" @click="connectWifi()" type="primary">1 连接设备AP</button>
		<button class="mini-btn" @click="tcpconnect()" type="primary">2 TCP连接</button>
		<button class="mini-btn" @click="tcpwrite()" type="primary">3 开始配网</button>
 
		<view>
			<text>{{ms}}</text>
		</view>
	</view>
</template>

<script>
	//创建一个 TCP Socket 实例
	const tcp = wx.createTCPSocket()
	export default {
		data() {
			return {
				wifissid: "",
				wifiBssid: "",
				APSSID: 'esp_device_AP',
				APpassword: 'hongwans',
				wifipassword: ""
			}
		},
		onLoad() {
			var that = this //这里声明了that；将this存在that里面 
			//初始化WiFi
			uni.startWifi({
				success(res) {
					console.log(res)
				},
				fail(err) {
					console.log(err)
				}
			});
			//读取连接的网络
			wx.getConnectedWifi({
				success: function(e) {
					console.log(e.wifi, 'wifi获取成功')
					if (e.wifi.SSID != that.APSSID) {
						that.wifissid = e.wifi.SSID;
						that.wifiBssid = e.wifi.BSSID;
					}
				},
				fail: function(e) {
					console.log(e, 'wifi获取失败')
				}
			});
		},
		methods: {
			onKeyInput: function(event) {
				var that = this //这里声明了that；将this存在that里面 
				that.wifipassword = event.target.value
			},
			onssidInput: function(event) {
				var that = this //这里声明了that；将this存在that里面 
				that.wifissid = event.target.values
			}, 
			
			//开始配网
			connectWifi() {
				var that = this //这里声明了that；将this存在that里面 
				wx.connectWifi({
					SSID: that.APSSID,
					password: that.APpassword,
					success(res) {
						uni.showToast({
							title: "连接设备AP成功",
							icon: 'success',
							duration: 2000
						});
						console.log(res)
					},
					fail(err) {
						console.log(err)
					}
				});
			},
			tcpconnect() {
				var that = this //这里声明了that；将this存在that里面  
				//启动连接
				tcp.connect({
					address: '192.168.1.1',
					port: 8899
				});

				tcp.onError(function(e) {
					uni.showToast({
						title: `TCP连接错误,请重试`,
						icon: 'none'
					})
				});

				//连接成功建立的时候触发该事件
				tcp.onConnect(function(e) {
					uni.showToast({
						title: `TCP连接成功`,
						icon: 'none'
					})
				});

			},

			tcpwrite() {
				var that = this //这里声明了that；将this存在that里面 
				if (that.wifipassword == "") {
					uni.showToast({
						title: `请输出WiFi密码`,
						icon: 'none'
					})
					return;
				}

				//发送数据 
				tcp.write("{\"ssid\":\"" + that.wifissid + "\",\"pass\":\"" + that.wifipassword + "\"}");
				uni.showToast({
					title: `发送数据成功`,
					icon: 'none'
				})

			},

		}
	}
</script>

<style>
	.content {
		display: flex;
		flex-direction: column;
		align-items: center;
		justify-content: center;
	}


	.uni-input {
		border: 1upx solid #000000;
		padding: 10upx 30upx;
		border-radius: 10upx;

	}

	.mini-btn {
		width: 80%;
		margin-top: 20px;
		line-height: 2;
	}

	.uni-form-item .title {
		padding: 20rpx 0;
	}
</style>