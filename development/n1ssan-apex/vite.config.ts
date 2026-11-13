import { readFileSync } from 'node:fs'
import tailwindcss from '@tailwindcss/vite'
import react from '@vitejs/plugin-react'
import { defineConfig } from 'vite'

// The release version lives in the repository root VERSION file, so Apex shows
// exactly the version of the build it is checking, like Armory and Loadout.
const appVersion = readFileSync(new URL('../../VERSION', import.meta.url), 'utf8').trim()

export default defineConfig({
  plugins: [react(), tailwindcss()],
  define: {
    __N1SSAN_VERSION__: JSON.stringify(appVersion),
  },
  // Tauri serves the frontend from a fixed port during development.
  server: {
    port: 5180,
    strictPort: true,
  },
  clearScreen: false,
  build: {
    target: 'chrome105',
  },
})
