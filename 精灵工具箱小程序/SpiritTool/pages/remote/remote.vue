<template>
	<view class="content">
		<view>
			<image class="logo" src="/static/开关.png" @click="goto()" @longpress="longpress" @touchend="touchend">
			</image>
		</view>

		<text>(短按开机，长按强制关机) </text>
		<view>

			<uni-card :title="device.id" :extra="device.online? '在线':'离线'">
				<view>
					<text class="uni-body" @click="LogOut()">设备名称: {{ device.title}}</text>
				</view>
				<view>
					<text class="uni-body">鉴权信息: {{ device.auth_info}}</text>
				</view>
				<view>
					<text class="uni-body">设备二维码信息：{"device_id":"xxx","api_key":"xxx"}</text>
				</view>
			</uni-card>

		</view>
		<button class="mini-btn" @click="senBleLabel()" type="primary">刷新</button>
		<view>
			<!-- 提示窗示例 -->
			<uni-popup ref="alertDialog" type="dialog">
				<uni-popup-dialog :type="msgType" cancelText="取消" confirmText="确认" title="系统提示" content="确定要移除设备?"
					@confirm="dialogConfirm"></uni-popup-dialog>
			</uni-popup>
		</view>
		<view>
			<!-- 提示窗示例 -->
			<uni-popup ref="force" type="dialog">
				<uni-popup-dialog :type="msgType" cancelText="取消" confirmText="确认" title="系统提示" content="确定要强制关机?"
					@confirm="forceConfirm"></uni-popup-dialog>
			</uni-popup>
		</view>

		<!-- 	<button class="mini-btn" @click="networking()" type="primary">设备配网</button> -->

	</view>
</template>

<script>
	//引用自定义js 公共方法api相关
	import until from '@/utils/api.js';
	export default {
		data() {
			return {
				islongPress: false, //长按记录变量
				msgType: 'success',
				device_id: "",
				api_key: "",
				device: {
					title: "没有绑定设备",
					online: false,
					id: "000000",
					auth_info: ""
				}
			}
		},
		async onLoad() {
			var that = this //这里声明了that；将this存在that里面  			
			const device_id = uni.getStorageSync('device_id');
			const api_key = uni.getStorageSync('api_key');
			if (!device_id) {
				uni.showToast({
					title: "没有绑定设备",
					icon: 'none',
					duration: 2000
				});
			} else {
				that.device_id = device_id;
				that.api_key = api_key;
				//查询设备信息
				var res = await until.OneENT_GetDevice(that.device_id, that.api_key);
				if (res != "") {
					this.device.title = res.title
					this.device.online = res.online
					this.device.id = res.id
					this.device.auth_info = res.auth_info
				}
			}
		},
		methods: {
			networking() {
				uni.navigateTo({
					url: '/pages/networking/networking',
				});
				console.log('ddd');
			},
			//刷新页面
			async senBleLabel() {
				var that = this //这里声明了that；将this存在that里面 

				const device_id = uni.getStorageSync('device_id');
				const api_key = uni.getStorageSync('api_key');
				if (!device_id) {
					uni.showToast({
						title: "没有绑定设备",
						icon: 'none',
						duration: 2000
					});
				} else {
					that.device_id = device_id;
					that.api_key = api_key;
					//查询设备信息
					var res = await until.OneENT_GetDevice(that.device_id, that.api_key);

					if (res != "") {
						console.log(res);
						this.device.title = res.title
						this.device.online = res.online
						this.device.id = res.id
						this.device.auth_info = res.auth_info
					} else {
						uni.showToast({
							title: "服务器连接错误",
							icon: 'error',
							duration: 2000
						});
					}
				}
			},
			//长按强制关机
			async longpress() {
				var that = this //这里声明了that；将this存在that里面 
				this.islongPress = true;

				var that = this //这里声明了that；将this存在that里面  
				that.msgType = 'warn'
				that.$refs.force.open();
			},
			//触摸延时
			touchend() {
				//延时执行为了防止 click() 还未判断 islongPress 的值就被置为 fasle
				setTimeout(() => {
					this.islongPress = false
				}, 200)
			},
			async goto() {
				var that = this //这里声明了that；将this存在that里面  
				if (this.islongPress == true) {
					console.log("短按中识别到了 长按事件");
					return;
				}
				//判断设备是否绑定  
				const device_id = uni.getStorageSync('device_id');
				const api_key = uni.getStorageSync('api_key');
				if (!device_id) {
					uni.showToast({
						title: "没有绑定设备",
						icon: 'none',
						duration: 2000
					});
					//如果没有绑定 打开摄像头 扫码二维码
					that.Scan();
					return;
				} else {
					that.device_id = device_id;
					that.api_key = api_key;
					//查询设备信息  
					var res = await until.OneENT_GetDevice(that.device_id, that.api_key);
					if (res != "err") {
						this.device.title = res.title
						this.device.online = res.online
						this.device.id = res.id
						this.device.auth_info = res.auth_info
					} else {
						uni.showToast({
							title: "服务器连接错误",
							icon: 'error',
							duration: 2000
						});
					}
				}

				//如果设备在线
				if (this.device.online) {
					var cmd = "{\"on\":\"true\"}";
					var res = await until.OneENT_SendCmd(that.device_id, that.api_key, cmd)

					if (res == "request:ok") {
						uni.showToast({
							title: "发送成功",
							icon: 'success',
							duration: 2000
						});
					} else {
						uni.showToast({
							title: "服务器连接错误",
							icon: 'error',
							duration: 2000
						});
					}
				} else {
					uni.showToast({
						title: "设备不在线",
						icon: 'error',
						duration: 2000
					});
				}
			},
			//扫码二维码
			Scan() {
				var that = this //这里声明了that；将this存在that里面  
				// 允许从相机和相册扫码
				uni.scanCode({
					scanType: ['qrCode'], //二维码
					success: function(res) {
						//分析扫码
						that.ScanCodeResult(res);
					}
				});
			},
			//处理扫描得到的二维码信息
			async ScanCodeResult(res) {
				var that = this //这里声明了that；将this存在that里面
				console.log('条码类型：' + res.scanType);
				console.log('条码内容：' + res.result);

				var CodeResult = res.result;
				var pagination = JSON.parse(CodeResult);

				uni.setStorageSync('device_id', pagination.device_id);
				uni.setStorageSync('api_key', pagination.api_key);

				that.senBleLabel();
			},
			//退出登录
			LogOut() {
				var that = this //这里声明了that；将this存在that里面  
				that.msgType = 'warn'
				that.$refs.alertDialog.open()
			},
			//退出登录 按钮点击后 反馈
			dialogConfirm() {
				var that = this //这里声明了that；将this存在that里面  
				//console.log('点击确认')  
				try {
					uni.removeStorageSync('device_id');
					uni.removeStorageSync('api_key');

					this.device.title = "没有绑定设备";
					this.device.online = false;
					this.device.id = "000000";
					this.device.auth_info = "";

				} catch (e) {
					// error
				}
			},
			//强制关机确认
			async forceConfirm() {
				var that = this //这里声明了that；将this存在that里面  
				//如果设备在线
				if (this.device.online) {
					var cmd = "{\"on\":\"false\"}";
					var res = await until.OneENT_SendCmd(that.device_id, that.api_key, cmd)
					if (res == "request:ok") {
						uni.showToast({
							title: "发送成功",
							icon: 'success',
							duration: 2000
						});
					} else {
						uni.showToast({
							title: "服务器连接错误",
							icon: 'error',
							duration: 2000
						});
					}
				} else {
					uni.showToast({
						title: "设备不在线",
						icon: 'error',
						duration: 2000
					});
				}
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

	.uni-card {
		box-shadow: none;
	}

	.uni-body {
		display: flex;
		flex-direction: column;
		align-items: center;
		justify-content: center;
		width: 500rpx;
	}

	.logo {
		height: 200rpx;
		width: 200rpx;
		margin-top: 100rpx;
		margin-left: auto;
		margin-right: auto;
		margin-bottom: 50rpx;
		border-radius: 50rpx;
		border: 5rpx solid #6699FF;
	}

	.mini-btn {
		width: 80%;
		margin-top: 20px;
		line-height: 2;
	}
</style>