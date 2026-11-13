import type { EntryResult } from '../lib/api'

type OffsetResultsProps = {
  entries: EntryResult[]
}

function formatRva(value: number | null) {
  if (value === null) return 'n/a'
  return `0x${value.toString(16).toUpperCase()}`
}

function toneFor(status: EntryResult['status']) {
  if (status === 'match') return 'text-apex-lime'
  if (status === 'moved') return 'text-apex-pink-soft'
  return 'text-apex-danger'
}

// One row per dataset entry: the entry name and the RVA found in client.dll.
// Moved entries are shown in pink, unresolved ones in red.
export function OffsetResults({ entries }: OffsetResultsProps) {
  return (
    <div className="results-scroll flex h-full w-full flex-col gap-[3px] overflow-y-auto">
      {entries.map((entry) => (
        <div
          key={entry.name}
          className="flex shrink-0 items-center justify-between rounded-md bg-white/[0.04] px-1.5 py-1"
          title={`${entry.status} | stored ${formatRva(entry.storedRva)} | found ${formatRva(entry.foundRva)}`}
        >
          <span className="truncate pr-2 text-[10px] text-white/80">{entry.name}</span>
          <span className={`font-mono text-[10px] tabular-nums ${toneFor(entry.status)}`}>{formatRva(entry.foundRva)}</span>
        </div>
      ))}
    </div>
  )
}
