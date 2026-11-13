type StatusPillProps = {
  connected: boolean
}

// Reflects whether cs2.exe is currently running with client.dll loaded.
export function StatusPill({ connected }: StatusPillProps) {
  return (
    <div className="flex items-center gap-1">
      <span
        className={`h-[5px] w-[5px] rounded-full transition-all duration-300 ${
          connected
            ? 'bg-apex-lime shadow-[0_0_8px_var(--color-apex-lime)]'
            : 'bg-apex-danger shadow-[0_0_6px_var(--color-apex-danger)]'
        }`}
      />
      <span className="text-[9px] text-apex-muted">{connected ? 'Active' : 'Offline'}</span>
    </div>
  )
}
