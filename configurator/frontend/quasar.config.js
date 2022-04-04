/* eslint-env node */

/*
 * This file runs in a Node context (it's NOT transpiled by Babel), so use only
 * the ES6 features that are supported by your Node version. https://node.green/
 */

// Configuration for your app
// https://v2.quasar.dev/quasar-cli-vite/quasar-config-js


const { configure } = require('quasar/wrappers');


module.exports = configure(function (/* ctx */) {
    return {
	eslint: {
	    // fix: true,
	    // include = [],
	    // exclude = [],
	    // rawOptions = {},
	    warnings: true,
	    errors: true
	},

	// https://v2.quasar.dev/quasar-cli-vite/prefetch-feature
	// preFetch: true,

	// app boot file (/src/boot)
	// --> boot files are part of "main.js"
	// https://v2.quasar.dev/quasar-cli-vite/boot-files
	boot: [

	],

	// https://v2.quasar.dev/quasar-cli-vite/quasar-config-js#css
	css: [
	    'app.scss'
	],

	// https://github.com/quasarframework/quasar/tree/dev/extras
	extras: [
	    // 'ionicons-v4',
	    // 'mdi-v5',
	    // 'fontawesome-v6',
	    // 'eva-icons',
	    // 'themify',
	    // 'line-awesome',
	    // 'roboto-font-latin-ext', // this or either 'roboto-font', NEVER both!

	    'roboto-font', // optional, you are not bound to it
	    'material-icons', // optional, you are not bound to it
	],

	// Full list of options: https://v2.quasar.dev/quasar-cli-vite/quasar-config-js#build
	build: {
	    target: {
		browser: [ 'es2019', 'edge88', 'firefox78', 'chrome87', 'safari13.1' ],
		node: 'node16'
	    },

	    vueRouterMode: 'hash', // available values: 'hash', 'history'
	    // vueRouterBase,
	    // vueDevtools,
	    // vueOptionsAPI: false,

	    // rebuildCache: true, // rebuilds Vite/linter/etc cache on startup

	    // publicPath: '/',
	    // analyze: true,
	    // env: {},
	    // rawDefine: {}
	    // ignorePublicFolder: true,
	    // minify: false,
	    // polyfillModulePreload: true,
	    // distDir

	    extendViteConf (viteConf) {
		console.log(viteConf)
	    },
	    // viteVuePluginOptions: {},


	    // vitePlugins: [
	    //   [ 'package-name', { ..options.. } ]
	    // ]
	},

	// Full list of options: https://v2.quasar.dev/quasar-cli-vite/quasar-config-js#devServer
	devServer: {
	    // https: true
	    open: true // opens browser window automatically
	},

	// https://v2.quasar.dev/quasar-cli-vite/quasar-config-js#framework
	framework: {
	    config: {},

	    // iconSet: 'material-icons', // Quasar icon set
	    // lang: 'en-US', // Quasar language pack

	    // For special cases outside of where the auto-import strategy can have an impact
	    // (like functional components as one of the examples),
	    // you can manually specify Quasar components/directives to be available everywhere:
	    //
	    // components: [],
	    // directives: [],

	    // Quasar plugins
	    plugins: []
	},

	// animations: 'all', // --- includes all animations
	// https://v2.quasar.dev/options/animations
	animations: [],

	// https://v2.quasar.dev/quasar-cli-vite/quasar-config-js#sourcefiles
	sourceFiles: {
	    rootComponent: 'src/App.vue',
	    router: 'src/router/index',
	    store: 'src/store/index',
	    //   registerServiceWorker: 'src-pwa/register-service-worker',
	    //   serviceWorker: 'src-pwa/custom-service-worker',
	    //   pwaManifestFile: 'src-pwa/manifest.json',
	    //   electronMain: 'src-electron/electron-main',
	    //   electronPreload: 'src-electron/electron-preload'
	},

	// https://v2.quasar.dev/quasar-cli-vite/developing-ssr/configuring-ssr
	ssr: {
	    // ssrPwaHtmlFilename: 'offline.html', // do NOT use index.html as name!
	    // will mess up SSR

	    // extendSSRWebserverConf (esbuildConf) {},
	    // extendPackageJson (json) {},

	    pwa: false,

	    // manualStoreHydration: true,
	    // manualPostHydrationTrigger: true,

	    prodPort: 3000, // The default port that the production server should use
	    // (gets superseded if process.env.PORT is specified at runtime)

	    middlewares: [
		'render' // keep this as last one
	    ]
	},

	// https://v2.quasar.dev/quasar-cli-vite/developing-pwa/configuring-pwa
	pwa: {
	    workboxMode: 'generateSW', // or 'injectManifest'
	    injectPwaMetaTags: true,
	    swFilename: 'sw.js',
	    manifestFilename: 'manifest.json',
	    useCredentialsForManifestTag: false,
	    // extendGenerateSWOptions (cfg) {}
	    // extendInjectManifestOptions (cfg) {},
	    // extendManifestJson (json) {}
	    // extendPWACustomSWConf (esbuildConf) {}
	},

	// Full list of options: https://v2.quasar.dev/quasar-cli-vite/developing-cordova-apps/configuring-cordova
	cordova: {
	    // noIosLegacyBuildFlag: true, // uncomment only if you know what you are doing
	},

	// Full list of options: https://v2.quasar.dev/quasar-cli-vite/developing-capacitor-apps/configuring-capacitor
	capacitor: {
	    hideSplashscreen: true
	},

	// Full list of options: https://v2.quasar.dev/quasar-cli-vite/developing-electron-apps/configuring-electron
	electron: {
	    // extendElectronMainConf (esbuildConf)
	    extendElectronPreloadConf (esbuildConf) {
		// Enable loading native .node files.
		// Source: https://github.com/evanw/esbuild/issues/1051#issuecomment-806325487
		const nativeNodeModulesPlugin = {
		    name: 'native-node-modules',
		    setup(build) {
			// If a ".node" file is imported within a module in the "file" namespace, resolve
			// it to an absolute path and put it into the "node-file" virtual namespace.
			build.onResolve({ filter: /\.node$/, namespace: 'file' }, args => ({
			    path: require.resolve(args.path, { paths: [args.resolveDir] }),
			    namespace: 'node-file',
			}))

			// Files in the "node-file" virtual namespace call "require()" on the
			// path from esbuild of the ".node" file in the output directory.
			build.onLoad({ filter: /.*/, namespace: 'node-file' }, args => ({
			    contents: `
	import path from ${JSON.stringify(args.path)}
	try { module.exports = require(path) }
	catch {}
      `,
			}))

			// If a ".node" file is imported within a module in the "node-file" namespace, put
			// it in the "file" namespace where esbuild's default loading behavior will handle
			// it. It is already an absolute path since we resolved it to one above.
			build.onResolve({ filter: /\.node$/, namespace: 'node-file' }, args => ({
			    path: args.path,
			    namespace: 'file',
			}))

			// Tell esbuild's default loading behavior to use the "file" loader for
			// these ".node" files.
			let opts = build.initialOptions
			opts.loader = opts.loader || {}
			opts.loader['.node'] = 'file'
		    },
		}

		esbuildConf.plugins.push(nativeNodeModulesPlugin)
	    },

	    inspectPort: 5858,

	    bundler: 'builder', // 'packager' or 'builder'

	    packager: {
		// https://github.com/electron-userland/electron-packager/blob/master/docs/api.md#options

		// OS X / Mac App Store
		// appBundleId: '',
		// appCategoryType: '',
		// osxSign: '',
		// protocol: 'myapp://path',

		// Windows only
		// win32metadata: { ... }
	    },

	    builder: {
		// https://www.electron.build/configuration/configuration

		appId: 'hs-configurator'
	    }
	},

	// Full list of options: https://v2.quasar.dev/quasar-cli-vite/developing-browser-extensions/configuring-bex
	bex: {
	    contentScripts: [
		'my-content-script'
	    ],

	    // extendBexScriptsConf (esbuildConf) {}
	    // extendBexManifestJson (json) {}
	}
    }
});
