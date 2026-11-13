import { useCallback, useEffect, useRef, useState } from 'react'
import { listen } from '@tauri-apps/api/event'
import { LiquidProgress } from './components/LiquidProgress'
import { OffsetResults } from './components/OffsetResults'
import { StatusPill } from './components/StatusPill'
import { WindowControls } from './components/WindowControls'
import {
  apexApply,
  apexScan,
  apexStatus,
  describeError,
  type ApexStatus,
  type EntryResult,
  type ProgressEvent,
  type ScanReport,
} from './lib/api'
import { startDrag } from './lib/window'

const APP_VERSION = __N1SSAN_VERSION__

type Phase = 'idle' | 'scanning' | 'found' | 'uptodate' | 'applying' | 'applied' | 'error'

const PHASE_MESSAGE: Record<Phase, string> = {
  idle: 'Ready to check offsets',
  scanning: 'Analyzing signatures...',
  found: '',
  uptodate: 'Offsets are up to date',
  applying: 'Writing new offsets...',
  applied: 'Successfully updated!',
  error: '',
}

export default function App() {
  const [status, setStatus] = useState<ApexStatus | null>(null)
  const [phase, setPhase] = useState<Phase>('idle')
  const [percent, setPercent] = useState(0)
  const [label, setLabel] = useState('STANDBY')
  const [report, setReport] = useState<ScanReport | null>(null)
  const [errorText, setErrorText] = useState('')
  const busy = phase === 'scanning' || phase === 'applying'
  const gameRunning = status?.gameRunning === true && status?.clientModule === true

  const busyRef = useRef(busy)
  busyRef.current = busy

  const refreshStatus = useCallback(async () => {
    try {
      setStatus(await apexStatus())
    } catch (error) {
      setPhase('error')
      setErrorText(describeError(error))
    }
  }, [])

  useEffect(() => {
    void refreshStatus()
    const timer = window.setInterval(() => {
      if (!busyRef.current) void refreshStatus()
    }, 2000)
    return () => window.clearInterval(timer)
  }, [refreshStatus])

  // The Rust side owns progress: every percentage shown is work that happened.
  useEffect(() => {
    const unlisten = listen<ProgressEvent>('apex-progress', (event) => {
      setPercent(event.payload.percent)
      setLabel(event.payload.label)
      if (event.payload.message) setErrorText('')
    })
    return () => {
      void unlisten.then((off) => off())
    }
  }, [])

  const entries: EntryResult[] = report?.entries ?? []

  const findOffsets = useCallback(async () => {
    setPhase('scanning')
    setErrorText('')
    setReport(null)
    setPercent(0)
    setLabel('SCANNING')
    try {
      const result = await apexScan()
      setReport(result)
      setPercent(100)
      setLabel('DONE')
      setPhase(result.moved + result.missing > 0 ? 'found' : 'uptodate')
    } catch (error) {
      setPercent(0)
      setLabel('FAILED')
      setPhase('error')
      setErrorText(describeError(error))
    }
  }, [])

  const updateOffsets = useCallback(async () => {
    setPhase('applying')
    setErrorText('')
    setPercent(0)
    setLabel('APPLYING')
    try {
      const result = await apexApply()
      setPercent(100)
      setLabel('SYNCED')
      setPhase('applied')
      setReport((current) =>
        current
          ? {
              ...current,
              moved: 0,
              entries: current.entries.map((entry) =>
                entry.status === 'moved' ? { ...entry, status: 'match', storedRva: entry.foundRva } : entry,
              ),
            }
          : current,
      )
      await refreshStatus()
      if (result.changed.length === 0) setErrorText('No entry changed; the dataset already matched the game.')
    } catch (error) {
      setPercent(0)
      setLabel('FAILED')
      setPhase('error')
      setErrorText(describeError(error))
    }
  }, [refreshStatus])

  const showResults = phase === 'found' && entries.length > 0
  const message = phase === 'error' ? errorText : PHASE_MESSAGE[phase]

  return (
    <div className="flex h-screen w-screen items-center justify-center bg-apex-shell p-2">
      <div
        className="relative flex h-full w-full flex-col overflow-hidden rounded-[32px] bg-apex-void p-2 shadow-[0_20px_40px_rgba(0,0,0,0.5),0_0_28px_rgba(120,30,160,0.22)]"
        onMouseDown={(event) => {
          if (event.button === 0) void startDrag()
        }}
      >
        <LiquidProgress percent={percent} label={label} />

        <div className="absolute inset-x-2 bottom-2 z-10 flex h-[185px] flex-col justify-between rounded-[24px] border border-apex-line bg-apex-panel px-2.5 py-2 shadow-[0_-4px_16px_rgba(0,0,0,0.4),0_10px_24px_rgba(0,0,0,0.6),0_0_20px_rgba(130,35,170,0.32)]">
          <div className="mx-auto mb-px h-0.5 w-6 rounded-full bg-white/20" />

          <div className="flex items-center justify-between">
            <div className="leading-tight">
              <span className="block text-[11px] font-semibold tracking-tight text-white/85">N1ssan Sync</span>
              <span className="block text-[7px] text-white/35">{APP_VERSION}</span>
            </div>
            <div className="flex items-center gap-1.5">
              <StatusPill connected={gameRunning} />
              <WindowControls />
            </div>
          </div>

          <div className="my-1 flex flex-1 items-center justify-center overflow-hidden">
            {showResults ? (
              <OffsetResults entries={entries} />
            ) : (
              <span
                className={`px-1 text-center text-[11px] font-medium ${
                  phase === 'applied' ? 'text-apex-lime' : phase === 'error' ? 'text-apex-danger' : 'text-white/75'
                }`}
              >
                {message}
              </span>
            )}
          </div>

          <div className="grid grid-cols-2 gap-1.5">
            <button
              type="button"
              disabled={busy || !gameRunning}
              onClick={() => void findOffsets()}
              className={`rounded-full border border-white/10 bg-[#080a09] px-1 py-1.5 text-[10px] font-semibold text-white shadow-[0_4px_7px_rgba(51,12,70,0.4)] transition-all disabled:cursor-not-allowed disabled:opacity-35 disabled:shadow-none disabled:text-white/40 ${
                phase === 'found' || phase === 'applied' || phase === 'uptodate'
                  ? ''
                  : 'border-apex-pink/60 bg-[#181c1a] shadow-[0_4px_16px_rgba(255,46,140,0.3),0_0_12px_rgba(130,35,170,0.45)]'
              } ${!busy && gameRunning ? 'hover:border-white/25 hover:bg-[#1c211f] active:scale-95' : ''}`}
            >
              Find Offset
            </button>
            <button
              type="button"
              disabled={busy || phase !== 'found'}
              onClick={() => void updateOffsets()}
              className={`rounded-full border border-white/10 bg-[#080a09] px-1 py-1.5 text-[10px] font-semibold text-white shadow-[0_4px_7px_rgba(51,12,70,0.4)] transition-all disabled:cursor-not-allowed disabled:opacity-35 disabled:shadow-none disabled:text-white/40 ${
                phase === 'found' ? 'border-apex-pink/60 bg-[#181c1a] shadow-[0_4px_16px_rgba(255,46,140,0.3),0_0_12px_rgba(130,35,170,0.45)]' : ''
              } ${!busy && phase === 'found' ? 'hover:border-white/25 hover:bg-[#1c211f] active:scale-95' : ''}`}
            >
              Update Offset
            </button>
          </div>
        </div>
      </div>
    </div>
  )
}
