---
layout: default
title: Data Visualization
---

# Data Visualization

Upload a CSV file to visualize your data and explore statistical summaries.

<div class="csv-viz-container">

  <div class="upload-section">
    <label for="csvFile" class="upload-label">
      <span class="upload-icon">📂</span>
      <span>Choose a CSV file or drag &amp; drop here</span>
    </label>
    <input type="file" id="csvFile" accept=".csv" />
  </div>

  <div id="controls" style="display:none;">

    <div class="section-card">
      <div class="card-title">Column Selection</div>
      <div class="control-row">
        <div class="ctrl-group">
          <label for="dtCol">Datetime column:</label>
          <select id="dtCol"><option value="">(none)</option></select>
        </div>
        <div class="ctrl-group">
          <label for="xCol">X-axis:</label>
          <select id="xCol"></select>
        </div>
        <div class="ctrl-group">
          <label for="yCol">Y-axis column(s):</label>
          <select id="yCol" multiple size="4"></select>
        </div>
        <div class="ctrl-group">
          <label for="chartType">Chart type:</label>
          <select id="chartType">
            <option value="line">Line</option>
            <option value="bar">Bar</option>
            <option value="scatter">Scatter</option>
          </select>
        </div>
      </div>
    </div>

    <div class="section-card" id="range-card" style="display:none;">
      <div class="card-title">Date &amp; Time Range Filter</div>
      <div class="control-row">
        <div class="ctrl-group">
          <label for="dtFrom">From:</label>
          <input type="datetime-local" id="dtFrom" />
        </div>
        <div class="ctrl-group">
          <label for="dtTo">To:</label>
          <input type="datetime-local" id="dtTo" />
        </div>
        <div class="ctrl-group align-end">
          <button id="resetRangeBtn" class="btn btn-sm btn-outline">Full Range</button>
        </div>
      </div>
      <div class="range-info" id="range-info"></div>
    </div>

    <div class="btn-row">
      <button id="plotBtn" class="btn">Plot &amp; Analyze</button>
      <button id="resetBtn" class="btn btn-outline">Reset</button>
    </div>

  </div>

  <div id="error-msg" class="error-msg"></div>

  <div id="chart-wrapper" style="display:none;" class="chart-wrapper">
    <canvas id="myChart"></canvas>
  </div>

  <div id="stats-section" style="display:none;">
    <h2>Statistical Analysis</h2>
    <p id="stats-subtitle" class="stats-subtitle"></p>
    <div id="stat-cards" class="stat-cards-grid"></div>
    <div class="daily-header">
      <h3>Per-Day Breakdown</h3>
      <div class="daily-col-select">
        <label for="statsYCol">Column:</label>
        <select id="statsYCol"></select>
      </div>
    </div>
    <div class="table-scroll">
      <table id="stats-table"></table>
    </div>
  </div>

  <div id="preview-section" style="display:none;">
    <h3>Data Preview <small id="row-count"></small></h3>
    <div class="table-scroll">
      <table id="preview-table"></table>
    </div>
  </div>

</div>

<style>
/* === Container === */
.csv-viz-container { margin-top: 24px; }

/* === Upload === */
.upload-section { position: relative; margin-bottom: 24px; }
.upload-label {
  display: flex; flex-direction: column; align-items: center; justify-content: center;
  gap: 10px; border: 2px dashed #155799; border-radius: 8px; padding: 40px 20px;
  cursor: pointer; color: #155799; font-weight: 600; font-size: 1rem;
  background: #f0f6ff; transition: background 0.2s, border-color 0.2s;
}
.upload-label:hover, .upload-label.drag-over {
  background: #dceeff; border-color: #159957; color: #159957;
}
.upload-icon { font-size: 2rem; }
#csvFile { position: absolute; inset: 0; opacity: 0; cursor: pointer; width: 100%; height: 100%; }

/* === Section cards === */
.section-card {
  border: 1px solid #d0dce8; border-radius: 8px; padding: 18px 20px;
  margin-bottom: 16px; background: #fafcff;
}
.card-title {
  font-size: 0.8rem; font-weight: 700; text-transform: uppercase;
  letter-spacing: 0.08em; color: #155799; margin-bottom: 14px;
}

/* === Controls === */
.control-row { display: flex; flex-wrap: wrap; align-items: flex-start; gap: 12px 20px; margin-bottom: 14px; }
.ctrl-group { display: flex; flex-direction: column; gap: 4px; }
.ctrl-group label { font-weight: 600; font-size: 0.88rem; color: #1a2b3c; }
.ctrl-group select,
.ctrl-group input[type="datetime-local"] {
  padding: 6px 10px; border: 1px solid #ccd6e0; border-radius: 4px;
  font-size: 0.9rem; background: #fff; color: #2d2d2d; min-width: 140px;
}
.ctrl-group select[multiple] { min-height: 90px; }
.align-end { justify-content: flex-end; padding-top: 20px; }
.range-info { font-size: 0.82rem; color: #555; margin-top: 8px; }

/* === Buttons === */
.btn-row { display: flex; gap: 10px; margin-bottom: 20px; }
.btn-sm { padding: 5px 12px !important; font-size: 0.82rem !important; }
.btn-outline {
  background: transparent !important; border: 2px solid #155799 !important; color: #155799 !important;
}
.btn-outline:hover { background: #155799 !important; color: #fff !important; }

/* === Error === */
.error-msg { color: #c0392b; font-size: 0.9rem; margin-bottom: 12px; min-height: 1.2em; }

/* === Chart === */
.chart-wrapper { position: relative; width: 100%; margin-bottom: 36px; }
#myChart { width: 100% !important; max-height: 460px; }

/* === Stats === */
#stats-section h2 { margin-bottom: 4px; }
.stats-subtitle { font-size: 0.88rem; color: #666; margin-bottom: 20px; }

.stat-cards-grid {
  display: grid; grid-template-columns: repeat(auto-fill, minmax(210px, 1fr));
  gap: 14px; margin-bottom: 32px;
}
.stat-card {
  background: linear-gradient(135deg, #eef4ff, #f6fff9);
  border: 1px solid #c8daf0; border-radius: 8px; padding: 16px 18px;
}
.stat-card-title {
  font-size: 0.78rem; font-weight: 700; text-transform: uppercase;
  letter-spacing: 0.07em; color: #155799; margin-bottom: 10px;
  white-space: nowrap; overflow: hidden; text-overflow: ellipsis;
}
.stat-metrics { display: grid; grid-template-columns: 1fr 1fr; gap: 8px; }
.stat-metric { display: flex; flex-direction: column; gap: 2px; }
.stat-metric-label { font-size: 0.72rem; color: #888; font-weight: 600; text-transform: uppercase; }
.stat-metric-value { font-size: 1.05rem; font-weight: 700; color: #1a2b3c; }
.stat-metric-value.stat-min  { color: #e74c3c; }
.stat-metric-value.stat-max  { color: #159957; }
.stat-metric-value.stat-mean { color: #155799; }

.daily-header { display: flex; align-items: center; flex-wrap: wrap; gap: 12px; margin: 28px 0 10px; }
.daily-header h3 { margin: 0; }
.daily-col-select { display: flex; align-items: center; gap: 8px; }
.daily-col-select label { font-weight: 600; font-size: 0.88rem; color: #1a2b3c; }
.daily-col-select select { padding: 5px 10px; border: 1px solid #ccd6e0; border-radius: 4px; font-size: 0.88rem; }

/* === Shared table === */
.table-scroll { overflow-x: auto; margin-bottom: 24px; }
.data-table { width: 100%; border-collapse: collapse; font-size: 0.88rem; }
.data-table th { background: #155799; color: #fff; padding: 9px 14px; text-align: left; white-space: nowrap; }
.data-table td { padding: 8px 14px; border-bottom: 1px solid #e0e0e0; white-space: nowrap; }
.data-table tr:nth-child(even) td { background: #f9fafb; }
.td-min  { color: #e74c3c; font-weight: 600; }
.td-max  { color: #159957; font-weight: 600; }
.td-mean { color: #155799; font-weight: 600; }

/* === Preview === */
#preview-section h3 { margin-top: 0; }
#preview-section small { font-size: 0.8rem; color: #888; font-weight: 400; }

@media (max-width: 600px) {
  .control-row { flex-direction: column; }
  .stat-cards-grid { grid-template-columns: 1fr 1fr; }
  .daily-header { flex-direction: column; align-items: flex-start; }
}
</style>

<script src="https://cdn.jsdelivr.net/npm/chart.js@4.4.3/dist/chart.umd.min.js"></script>
<script>
(function () {
  'use strict';

  /* ── DOM refs ── */
  const fileInput     = document.getElementById('csvFile');
  const uploadLabel   = document.querySelector('.upload-label');
  const controls      = document.getElementById('controls');
  const dtColSel      = document.getElementById('dtCol');
  const xColSel       = document.getElementById('xCol');
  const yColSel       = document.getElementById('yCol');
  const chartTypeSel  = document.getElementById('chartType');
  const plotBtn       = document.getElementById('plotBtn');
  const resetBtn      = document.getElementById('resetBtn');
  const rangeCard     = document.getElementById('range-card');
  const dtFrom        = document.getElementById('dtFrom');
  const dtTo          = document.getElementById('dtTo');
  const resetRangeBtn = document.getElementById('resetRangeBtn');
  const rangeInfo     = document.getElementById('range-info');
  const errorMsg      = document.getElementById('error-msg');
  const chartWrapper  = document.getElementById('chart-wrapper');
  const chartCanvas   = document.getElementById('myChart');
  const statsSec      = document.getElementById('stats-section');
  const statsSubtitle = document.getElementById('stats-subtitle');
  const statCards     = document.getElementById('stat-cards');
  const statsYColSel  = document.getElementById('statsYCol');
  const statsTable    = document.getElementById('stats-table');
  const previewSec    = document.getElementById('preview-section');
  const previewTbl    = document.getElementById('preview-table');
  const rowCount      = document.getElementById('row-count');

  let parsedHeaders = [];
  let parsedRows    = [];
  let dtMin = null, dtMax = null;
  let chartInstance = null;

  const PALETTE = [
    'rgba(21,87,153,0.8)',  'rgba(21,153,87,0.8)',
    'rgba(231,76,60,0.8)',  'rgba(243,156,18,0.8)',
    'rgba(142,68,173,0.8)', 'rgba(52,152,219,0.8)',
    'rgba(26,188,156,0.8)', 'rgba(230,126,34,0.8)',
  ];

  /* ── Drag-and-drop ── */
  uploadLabel.addEventListener('dragover', e => { e.preventDefault(); uploadLabel.classList.add('drag-over'); });
  uploadLabel.addEventListener('dragleave', () => uploadLabel.classList.remove('drag-over'));
  uploadLabel.addEventListener('drop', e => {
    e.preventDefault();
    uploadLabel.classList.remove('drag-over');
    if (e.dataTransfer.files[0]) handleFile(e.dataTransfer.files[0]);
  });
  fileInput.addEventListener('change', () => { if (fileInput.files[0]) handleFile(fileInput.files[0]); });

  /* ── RFC 4180 CSV parser ── */
  function parseCSV(text) {
    const lines = text.replace(/\r\n/g, '\n').replace(/\r/g, '\n').split('\n');
    const result = [];
    for (const line of lines) {
      if (!line.trim()) continue;
      const row = [];
      let inQuote = false, cell = '';
      for (let i = 0; i < line.length; i++) {
        const ch = line[i];
        if (ch === '"') {
          if (inQuote && line[i+1] === '"') { cell += '"'; i++; }
          else inQuote = !inQuote;
        } else if (ch === ',' && !inQuote) { row.push(cell.trim()); cell = ''; }
        else cell += ch;
      }
      row.push(cell.trim());
      result.push(row);
    }
    return result;
  }

  /* ── Datetime helpers ── */
  function toDatetimeLocal(date) {
    const off = date.getTimezoneOffset() * 60000;
    return new Date(date - off).toISOString().slice(0, 16);
  }
  function fromDatetimeLocal(str) { return str ? new Date(str) : null; }
  function toLocalDateStr(date)    { return date.toLocaleDateString('en-CA'); } // YYYY-MM-DD

  /* ── Auto-detect datetime columns ── */
  const DT_KEYWORDS = /date|time|timestamp|created|updated|recorded|measured|at$/i;
  function detectDatetimeCols(headers, rows) {
    return headers.filter((h, i) => {
      if (DT_KEYWORDS.test(h)) return true;
      const sample = rows.slice(0, 5).map(r => r[i]).filter(v => v && v.length > 6);
      return sample.length > 0 && sample.every(v => !isNaN(Date.parse(v)) && isNaN(parseFloat(v)));
    });
  }

  /* ── File handling ── */
  function handleFile(file) {
    errorMsg.textContent = '';
    if (!file.name.toLowerCase().endsWith('.csv')) { showError('Please upload a .csv file.'); return; }
    const reader = new FileReader();
    reader.onload  = e => { try { processCSV(e.target.result); } catch (err) { showError('Could not parse: ' + err.message); } };
    reader.onerror = () => showError('Failed to read the file.');
    reader.readAsText(file);
  }

  function processCSV(text) {
    const rows = parseCSV(text);
    if (rows.length < 2) { showError('CSV must have a header row and at least one data row.'); return; }

    parsedHeaders = rows[0];
    parsedRows    = rows.slice(1);

    // Datetime column selector
    dtColSel.innerHTML = '<option value="">(none)</option>';
    const dtCols = detectDatetimeCols(parsedHeaders, parsedRows);
    parsedHeaders.forEach(h => {
      const opt = document.createElement('option');
      opt.value = opt.textContent = h;
      dtColSel.appendChild(opt);
    });
    if (dtCols.length > 0) dtColSel.value = dtCols[0];

    populateSelect(xColSel, parsedHeaders);
    populateSelect(yColSel, parsedHeaders);
    xColSel.value = parsedHeaders[0];
    Array.from(yColSel.options).forEach((opt, i) => { opt.selected = i > 0; });

    controls.style.display = '';
    onDtColChange();
    renderPreview();
  }

  function populateSelect(sel, headers) {
    sel.innerHTML = '';
    headers.forEach(h => {
      const opt = document.createElement('option');
      opt.value = opt.textContent = h;
      sel.appendChild(opt);
    });
  }

  /* ── Datetime column change ── */
  dtColSel.addEventListener('change', onDtColChange);

  function onDtColChange() {
    const dtCol = dtColSel.value;
    if (!dtCol || !parsedRows.length) { rangeCard.style.display = 'none'; return; }

    const dtIdx = parsedHeaders.indexOf(dtCol);
    const dates  = parsedRows.map(r => new Date(r[dtIdx])).filter(d => !isNaN(d));
    if (!dates.length) { rangeCard.style.display = 'none'; return; }

    dtMin = new Date(Math.min(...dates));
    dtMax = new Date(Math.max(...dates));
    dtFrom.value = toDatetimeLocal(dtMin);
    dtTo.value   = toDatetimeLocal(dtMax);
    updateRangeInfo();
    rangeCard.style.display = '';
  }

  resetRangeBtn.addEventListener('click', () => {
    if (dtMin) dtFrom.value = toDatetimeLocal(dtMin);
    if (dtMax) dtTo.value   = toDatetimeLocal(dtMax);
    updateRangeInfo();
  });
  dtFrom.addEventListener('change', updateRangeInfo);
  dtTo.addEventListener('change',   updateRangeInfo);

  function updateRangeInfo() {
    const from = fromDatetimeLocal(dtFrom.value);
    const to   = fromDatetimeLocal(dtTo.value);
    if (!from || !to) { rangeInfo.textContent = ''; return; }
    const days = Math.round((to - from) / 86400000);
    rangeInfo.textContent = `${from.toLocaleString()} → ${to.toLocaleString()} (${days} day${days !== 1 ? 's' : ''})`;
  }

  /* ── Filtered rows ── */
  function getFilteredRows() {
    const dtCol = dtColSel.value;
    if (!dtCol) return parsedRows;
    const dtIdx = parsedHeaders.indexOf(dtCol);
    const from  = fromDatetimeLocal(dtFrom.value);
    const to    = fromDatetimeLocal(dtTo.value);
    if (!from || !to) return parsedRows;
    return parsedRows.filter(r => {
      const d = new Date(r[dtIdx]);
      return !isNaN(d) && d >= from && d <= to;
    });
  }

  /* ── Plot button ── */
  plotBtn.addEventListener('click', () => {
    errorMsg.textContent = '';
    const xCol  = xColSel.value;
    const yCols = Array.from(yColSel.selectedOptions).map(o => o.value);
    const type  = chartTypeSel.value;
    if (!xCol)         { showError('Please select an X-axis column.'); return; }
    if (!yCols.length) { showError('Please select at least one Y-axis column.'); return; }

    const rows = getFilteredRows();
    if (!rows.length)  { showError('No data in the selected date/time range.'); return; }

    renderChart(rows, xCol, yCols, type);
    renderStats(rows, yCols);
  });

  /* ── Chart ── */
  function renderChart(rows, xCol, yCols, type) {
    const xIdx   = parsedHeaders.indexOf(xCol);
    const labels = rows.map(r => r[xIdx]);

    const datasets = yCols.map((yCol, i) => {
      const yIdx  = parsedHeaders.indexOf(yCol);
      const color = PALETTE[i % PALETTE.length];
      const data  = rows.map(r => { const v = parseFloat(r[yIdx]); return isNaN(v) ? null : v; });

      if (type === 'scatter') {
        return {
          label: yCol,
          data: labels.map((lbl, j) => ({ x: parseFloat(lbl) || lbl, y: data[j] })),
          backgroundColor: color, borderColor: color, pointRadius: 4,
        };
      }
      return {
        label: yCol, data,
        backgroundColor: type === 'bar' ? color : color.replace('0.8', '0.15'),
        borderColor: color.replace('0.8', '1'),
        borderWidth: 2,
        pointRadius: type === 'line' ? 3 : undefined,
        fill: type === 'line',
        tension: 0.3,
      };
    });

    if (chartInstance) chartInstance.destroy();
    chartWrapper.style.display = '';
    chartInstance = new Chart(chartCanvas, {
      type,
      data: type === 'scatter' ? { datasets } : { labels, datasets },
      options: {
        responsive: true,
        animation: { duration: 400 },
        plugins: {
          legend: { position: 'top' },
          tooltip: { mode: 'index', intersect: false },
        },
        scales: {
          x: {
            title: { display: true, text: xCol, font: { weight: 'bold' } },
            ticks: { maxTicksLimit: 20, maxRotation: 45 },
          },
          y: {
            title: { display: true, text: yCols.join(' / '), font: { weight: 'bold' } },
            beginAtZero: false,
          },
        },
      },
    });
  }

  /* ── Statistics helpers ── */
  function computeStats(nums) {
    if (!nums.length) return null;
    const sum  = nums.reduce((a, b) => a + b, 0);
    const mean = sum / nums.length;
    const min  = Math.min(...nums);
    const max  = Math.max(...nums);
    const variance = nums.reduce((a, b) => a + (b - mean) ** 2, 0) / nums.length;
    return { count: nums.length, min, max, mean, stdDev: Math.sqrt(variance) };
  }

  function getNumericValues(rows, colName) {
    const idx = parsedHeaders.indexOf(colName);
    return rows.map(r => parseFloat(r[idx])).filter(v => !isNaN(v));
  }

  /* ── Stats section ── */
  function renderStats(rows, yCols) {
    const dtCol = dtColSel.value;
    if (dtCol) {
      const from = fromDatetimeLocal(dtFrom.value);
      const to   = fromDatetimeLocal(dtTo.value);
      statsSubtitle.textContent =
        `${rows.length} rows · ${from ? from.toLocaleDateString() : ''} – ${to ? to.toLocaleDateString() : ''}`;
    } else {
      statsSubtitle.textContent = `${rows.length} rows`;
    }

    /* Overall stat cards — one per Y column */
    statCards.innerHTML = '';
    yCols.forEach(yCol => {
      const s = computeStats(getNumericValues(rows, yCol));
      if (!s) return;
      statCards.insertAdjacentHTML('beforeend', `
        <div class="stat-card">
          <div class="stat-card-title" title="${escHtml(yCol)}">${escHtml(yCol)}</div>
          <div class="stat-metrics">
            <div class="stat-metric">
              <span class="stat-metric-label">Min</span>
              <span class="stat-metric-value stat-min">${fmt(s.min)}</span>
            </div>
            <div class="stat-metric">
              <span class="stat-metric-label">Max</span>
              <span class="stat-metric-value stat-max">${fmt(s.max)}</span>
            </div>
            <div class="stat-metric">
              <span class="stat-metric-label">Mean</span>
              <span class="stat-metric-value stat-mean">${fmt(s.mean)}</span>
            </div>
            <div class="stat-metric">
              <span class="stat-metric-label">Std Dev</span>
              <span class="stat-metric-value">${fmt(s.stdDev)}</span>
            </div>
          </div>
        </div>`);
    });

    /* Per-day column selector */
    statsYColSel.innerHTML = '';
    yCols.forEach(yCol => {
      const opt = document.createElement('option');
      opt.value = opt.textContent = yCol;
      statsYColSel.appendChild(opt);
    });
    renderDailyTable(rows, yCols[0]);
    statsYColSel.onchange = () => renderDailyTable(rows, statsYColSel.value);

    statsSec.style.display = '';
  }

  /* ── Per-day table ── */
  function renderDailyTable(rows, yCol) {
    const dtCol = dtColSel.value;
    const yIdx  = parsedHeaders.indexOf(yCol);
    const groups = {};

    rows.forEach(r => {
      let dayKey;
      if (dtCol) {
        const dtIdx = parsedHeaders.indexOf(dtCol);
        const d = new Date(r[dtIdx]);
        dayKey = isNaN(d) ? '(unknown)' : toLocalDateStr(d);
      } else {
        dayKey = 'All Data';
      }
      const v = parseFloat(r[yIdx]);
      if (!isNaN(v)) {
        if (!groups[dayKey]) groups[dayKey] = [];
        groups[dayKey].push(v);
      }
    });

    const days = Object.keys(groups).sort();
    let html = `<thead><tr>
      <th>Date</th><th>Count</th><th>Min</th><th>Mean</th><th>Max</th><th>Std Dev</th>
    </tr></thead><tbody>`;
    days.forEach(day => {
      const s = computeStats(groups[day]);
      if (!s) return;
      html += `<tr>
        <td>${escHtml(day)}</td>
        <td>${s.count}</td>
        <td class="td-min">${fmt(s.min)}</td>
        <td class="td-mean">${fmt(s.mean)}</td>
        <td class="td-max">${fmt(s.max)}</td>
        <td>${fmt(s.stdDev)}</td>
      </tr>`;
    });
    html += '</tbody>';
    statsTable.className = 'data-table';
    statsTable.innerHTML = html;
  }

  /* ── Data preview ── */
  function renderPreview() {
    previewSec.style.display = '';
    const maxRows = Math.min(parsedRows.length, 10);
    rowCount.textContent = `(${parsedRows.length} rows — showing first ${maxRows})`;
    let html = '<thead><tr>';
    parsedHeaders.forEach(h => { html += `<th>${escHtml(h)}</th>`; });
    html += '</tr></thead><tbody>';
    for (let i = 0; i < maxRows; i++) {
      html += '<tr>';
      parsedRows[i].forEach(cell => { html += `<td>${escHtml(cell)}</td>`; });
      html += '</tr>';
    }
    html += '</tbody>';
    previewTbl.className = 'data-table';
    previewTbl.innerHTML = html;
  }

  /* ── Reset ── */
  resetBtn.addEventListener('click', () => {
    if (chartInstance) { chartInstance.destroy(); chartInstance = null; }
    controls.style.display     = 'none';
    chartWrapper.style.display = 'none';
    statsSec.style.display     = 'none';
    previewSec.style.display   = 'none';
    rangeCard.style.display    = 'none';
    errorMsg.textContent       = '';
    fileInput.value            = '';
    parsedHeaders = []; parsedRows = []; dtMin = null; dtMax = null;
  });

  /* ── Utilities ── */
  function fmt(n) {
    return typeof n === 'number'
      ? n.toLocaleString(undefined, { maximumFractionDigits: 4 })
      : String(n);
  }
  function showError(msg) { errorMsg.textContent = msg; }
  function escHtml(str) {
    return String(str)
      .replace(/&/g, '&amp;').replace(/</g, '&lt;')
      .replace(/>/g, '&gt;').replace(/"/g, '&quot;');
  }
})();
</script>
