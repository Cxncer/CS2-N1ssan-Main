import { getCurrentWindow } from '@tauri-apps/api/window'

// Frameless window controls: the shell has no title bar, so the dashboard
// header carries the minimize and close actions.
export async function minimizeWindow() {
  await getCurrentWindow().minimize()
}

export async function closeWindow() {
  await getCurrentWindow().close()
}

export function startDrag() {
  void getCurrentWindow().startDragging()
}
