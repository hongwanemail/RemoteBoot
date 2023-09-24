<template>
	<view class="container">

		<view>
			<image class="logo" src="/static/创客宏万.png" @click="goto()">
			</image>
		</view>

		<view>
			<uni-section title="功能菜单" type="line" padding>
				<uni-grid :column="3" :showBorder="false" :square="false" :highlight="false" @change="change">
					<uni-grid-item v-for="(item, index) in Menulist" :index="index" :key="index">
						<view class="grid-item-box">
							<image class="image" :src="item.url" mode="aspectFill" />

							<text class="text">{{ item.text }}</text>
							<view v-if="item.badge" class="grid-dot">
								<uni-badge :text="item.badge" :type="item.type" />
							</view>
						</view>
					</uni-grid-item>
				</uni-grid>
			</uni-section>

		</view>



	</view>
</template>

<script>
	//引用自定义js 公共方法api相关
	import until from '@/utils/api.js';
	export default {
		data() {
			return {
				Menulist: [{
						url: '/static/开关.png',
						text: '远程开关机'
					},
					{
						url: '/static/网络配置.png',
						text: '设备配网'
					},
					{
						url: '/static/敬请期待.png',
						text: '敬请期待...'
					}

				]
			}
		},
		async onLoad() {
			var that = this //这里声明了that；将this存在that里面  			

		},
		methods: {
			change(e) {
				let {
					index
				} = e.detail

				switch (index + 1) {
					case 1:
						uni.navigateTo({
							url: '/pages/remote/remote',
						});

						break;
					case 2:
						uni.navigateTo({
							url: '/pages/networking/networking',
						});
						break;
					case 3:
						uni.showToast({
							title: `敬请期待...`,
							icon: 'none'
						})
						break;
					default:
						break;
				}


			},
			goto() {
				uni.showToast({
					title: `创客宏万感谢您的支持`,
					icon: 'none'
				})
			}
		}
	}
</script>

<style>
	.container {
		padding: 5px;
		font-size: 14px;
		line-height: 24px;
		 
	 
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
		height: 300rpx;
		width: 700rpx;
		margin-top: 50rpx;
		margin-left: 10rpx;
		margin-right: auto;
		margin-bottom: 50rpx;
		border-radius: 20rpx; 
	}

	.mini-btn {
		width: 80%;
		margin-top: 20px;
		line-height: 2;
	}

	.image {
		width: 25px;
		height: 25px;
	}

	.text {
		font-size: 14px;
		margin-top: 5px;
	}

	.grid-item-box {
		flex: 1;
		// position: relative;
		/* #ifndef APP-NVUE */
		display: flex;
		/* #endif */
		flex-direction: column;
		align-items: center;
		justify-content: center;
		padding: 15px 0;
	}
</style>