import { closeWindow, minimizeWindow } from '../lib/window'

// The window is frameless, so the shell owns its minimize/close affordances.
export function WindowControls() {
  return (
    <div className="ml-1 flex items-center gap-1.5">
      <button
        type="button"
        title="Minimize"
        aria-label="Minimize"
        onClick={() => void minimizeWindow()}
        className="flex items-center justify-center text-white/85 transition-opacity hover:text-white"
      >
        <svg width="9" height="9" viewBox="0 0 12 12" fill="none" stroke="currentColor" strokeWidth="1.8" strokeLinecap="round">
          <line x1="2" y1="6" x2="10" y2="6" />
        </svg>
      </button>
      <button
        type="button"
        title="Close"
        aria-label="Close"
        onClick={() => void closeWindow()}
        className="flex items-center justify-center text-white/85 transition-opacity hover:text-white"
      >
        <svg width="9" height="9" viewBox="0 0 12 12" fill="none" stroke="currentColor" strokeWidth="1.8" strokeLinecap="round">
          <line x1="2.5" y1="2.5" x2="9.5" y2="9.5" />
          <line x1="9.5" y1="2.5" x2="2.5" y2="9.5" />
        </svg>
      </button>
    </div>
  )
}
