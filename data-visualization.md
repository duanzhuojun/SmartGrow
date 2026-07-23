---
layout: default
title: Data Visualization
---

# Data Visualization

Upload a CSV file to visualize your data as an interactive chart.

<div class="csv-viz-container">

  <div class="upload-section">
    <label for="csvFile" class="upload-label">
      <span class="upload-icon">📂</span>
      <span>Choose a CSV file or drag &amp; drop here</span>
    </label>
    <input type="file" id="csvFile" accept=".csv" />
  </div>

  <div id="controls" style="display:none;">
    <div class="control-row">
      <label for="xCol">X-axis column:</label>
      <select id="xCol"></select>

      <label for="yCol">Y-axis column(s):</label>
      <select id="yCol" multiple size="4"></select>

      <label for="chartType">Chart type:</label>
      <select id="chartType">
        <option value="line">Line</option>
        <option value="bar">Bar</option>
        <option value="scatter">Scatter</option>
      </select>
    </div>
    <button id="plotBtn" class="btn">Plot</button>
    <button id="resetBtn" class="btn btn-outline">Reset</button>
  </div>

  <div id="error-msg" class="error-msg"></div>

  <div class="chart-wrapper">
    <canvas id="myChart"></canvas>
  </div>

  <div id="preview-section" style="display:none;">
    <h3>Data Preview <small id="row-count"></small></h3>
    <div class="table-scroll">
      <table id="preview-table"></table>
    </div>
  </div>

</div>

<style>
.csv-viz-container {
  margin-top: 24px;
}

/* Upload zone */
.upload-section {
  position: relative;
  margin-bottom: 24px;
}
.upload-label {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  gap: 10px;
  border: 2px dashed #155799;
  border-radius: 8px;
  padding: 40px 20px;
  cursor: pointer;
  color: #155799;
  font-weight: 600;
  font-size: 1rem;
  background: #f0f6ff;
  transition: background 0.2s, border-color 0.2s;
}
.upload-label:hover, .upload-label.drag-over {
  background: #dceeff;
  border-color: #159957;
  color: #159957;
}
.upload-icon { font-size: 2rem; }
#csvFile {
  position: absolute;
  inset: 0;
  opacity: 0;
  cursor: pointer;
  width: 100%;
  height: 100%;
}

/* Controls */
.control-row {
  display: flex;
  flex-wrap: wrap;
  align-items: flex-start;
  gap: 12px 20px;
  margin-bottom: 14px;
}
.control-row label {
  font-weight: 600;
  font-size: 0.9rem;
  color: #1a2b3c;
  align-self: center;
}
.control-row select {
  padding: 6px 10px;
  border: 1px solid #ccd6e0;
  border-radius: 4px;
  font-size: 0.9rem;
  background: #fff;
  color: #2d2d2d;
  min-width: 120px;
}
.control-row select[multiple] { min-height: 90px; }
.btn-outline {
  background: transparent !important;
  border: 2px solid #155799 !important;
  color: #155799 !important;
}
.btn-outline:hover { background: #155799 !important; color: #fff !important; }

/* Error */
.error-msg {
  color: #c0392b;
  font-size: 0.9rem;
  margin-bottom: 12px;
  min-height: 1.2em;
}

/* Chart */
.chart-wrapper {
  position: relative;
  width: 100%;
  max-height: 480px;
  margin-bottom: 32px;
}
#myChart { width: 100% !important; }

/* Preview table */
.table-scroll {
  overflow-x: auto;
  margin-top: 10px;
}
#preview-table {
  width: 100%;
  border-collapse: collapse;
  font-size: 0.85rem;
}
#preview-table th {
  background: #155799;
  color: #fff;
  padding: 8px 12px;
  text-align: left;
  white-space: nowrap;
}
#preview-table td {
  padding: 7px 12px;
  border-bottom: 1px solid #e0e0e0;
  white-space: nowrap;
}
#preview-table tr:nth-child(even) td { background: #f9fafb; }
#preview-section h3 { margin-top: 0; }
#preview-section small { font-size: 0.8rem; color: #888; font-weight: 400; }

@media (max-width: 600px) {
  .control-row { flex-direction: column; }
}
</style>

<script src="https://cdn.jsdelivr.net/npm/chart.js@4.4.3/dist/chart.umd.min.js"></script>
<script>
(function () {
  'use strict';

  /* ── DOM refs ── */
  const fileInput    = document.getElementById('csvFile');
  const uploadLabel  = document.querySelector('.upload-label');
  const controls    = document.getElementById('controls');
  const xColSel     = document.getElementById('xCol');
  const yColSel     = document.getElementById('yCol');
  const chartTypeSel = document.getElementById('chartType');
  const plotBtn     = document.getElementById('plotBtn');
  const resetBtn    = document.getElementById('resetBtn');
  const errorMsg    = document.getElementById('error-msg');
  const chartCanvas = document.getElementById('myChart');
  const previewSec  = document.getElementById('preview-section');
  const previewTbl  = document.getElementById('preview-table');
  const rowCount    = document.getElementById('row-count');

  let parsedHeaders = [];
  let parsedRows    = [];
  let chartInstance = null;

  /* ── Colour palette ── */
  const PALETTE = [
    'rgba(21,87,153,0.8)',   'rgba(21,153,87,0.8)',
    'rgba(231,76,60,0.8)',   'rgba(243,156,18,0.8)',
    'rgba(142,68,173,0.8)',  'rgba(52,152,219,0.8)',
    'rgba(26,188,156,0.8)',  'rgba(230,126,34,0.8)',
  ];

  /* ── Drag-and-drop ── */
  uploadLabel.addEventListener('dragover', e => { e.preventDefault(); uploadLabel.classList.add('drag-over'); });
  uploadLabel.addEventListener('dragleave', () => uploadLabel.classList.remove('drag-over'));
  uploadLabel.addEventListener('drop', e => {
    e.preventDefault();
    uploadLabel.classList.remove('drag-over');
    const file = e.dataTransfer.files[0];
    if (file) handleFile(file);
  });

  fileInput.addEventListener('change', () => {
    if (fileInput.files[0]) handleFile(fileInput.files[0]);
  });

  /* ── CSV parser (RFC 4180-compatible) ── */
  function parseCSV(text) {
    const lines = text.replace(/\r\n/g, '\n').replace(/\r/g, '\n').split('\n');
    const result = [];
    for (const line of lines) {
      if (line.trim() === '') continue;
      const row = [];
      let inQuote = false;
      let cell = '';
      for (let i = 0; i < line.length; i++) {
        const ch = line[i];
        if (ch === '"') {
          if (inQuote && line[i + 1] === '"') { cell += '"'; i++; }
          else inQuote = !inQuote;
        } else if (ch === ',' && !inQuote) {
          row.push(cell.trim());
          cell = '';
        } else {
          cell += ch;
        }
      }
      row.push(cell.trim());
      result.push(row);
    }
    return result;
  }

  function handleFile(file) {
    errorMsg.textContent = '';
    if (!file.name.toLowerCase().endsWith('.csv')) {
      showError('Please upload a .csv file.');
      return;
    }
    const reader = new FileReader();
    reader.onload = e => {
      try {
        processCSV(e.target.result);
      } catch (err) {
        showError('Could not parse file: ' + err.message);
      }
    };
    reader.onerror = () => showError('Failed to read the file.');
    reader.readAsText(file);
  }

  function processCSV(text) {
    const rows = parseCSV(text);
    if (rows.length < 2) { showError('CSV must have a header row and at least one data row.'); return; }

    parsedHeaders = rows[0];
    parsedRows    = rows.slice(1);

    /* Populate column selectors */
    populateSelect(xColSel, parsedHeaders, false);
    populateSelect(yColSel, parsedHeaders, true);

    /* Auto-select first col for X, remaining for Y */
    xColSel.value = parsedHeaders[0];
    Array.from(yColSel.options).forEach((opt, i) => { opt.selected = i > 0; });

    controls.style.display = '';
    renderPreview();
  }

  function populateSelect(sel, headers, isMulti) {
    sel.innerHTML = '';
    headers.forEach(h => {
      const opt = document.createElement('option');
      opt.value = opt.textContent = h;
      sel.appendChild(opt);
    });
  }

  /* ── Preview table ── */
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
    previewTbl.innerHTML = html;
  }

  /* ── Plot ── */
  plotBtn.addEventListener('click', renderChart);

  function renderChart() {
    errorMsg.textContent = '';
    const xCol  = xColSel.value;
    const yCols = Array.from(yColSel.selectedOptions).map(o => o.value);
    const type  = chartTypeSel.value;

    if (!xCol)           { showError('Please select an X-axis column.'); return; }
    if (!yCols.length)   { showError('Please select at least one Y-axis column.'); return; }

    const xIdx = parsedHeaders.indexOf(xCol);
    const labels = parsedRows.map(r => r[xIdx]);

    const datasets = yCols.map((yCol, i) => {
      const yIdx = parsedHeaders.indexOf(yCol);
      const color = PALETTE[i % PALETTE.length];
      const data = parsedRows.map(r => {
        const v = parseFloat(r[yIdx]);
        return isNaN(v) ? null : v;
      });

      if (type === 'scatter') {
        return {
          label: yCol,
          data: labels.map((lbl, j) => ({ x: parseFloat(lbl) || lbl, y: data[j] })),
          backgroundColor: color,
          borderColor: color,
          pointRadius: 5,
        };
      }
      return {
        label: yCol,
        data,
        backgroundColor: type === 'bar' ? color : color.replace('0.8', '0.15'),
        borderColor: color.replace('0.8', '1'),
        borderWidth: 2,
        pointRadius: type === 'line' ? 3 : undefined,
        fill: type === 'line',
        tension: 0.3,
      };
    });

    if (chartInstance) chartInstance.destroy();

    chartInstance = new Chart(chartCanvas, {
      type,
      data: type === 'scatter' ? { datasets } : { labels, datasets },
      options: {
        responsive: true,
        animation: { duration: 500 },
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

  /* ── Reset ── */
  resetBtn.addEventListener('click', () => {
    if (chartInstance) { chartInstance.destroy(); chartInstance = null; }
    controls.style.display   = 'none';
    previewSec.style.display = 'none';
    errorMsg.textContent     = '';
    fileInput.value          = '';
    parsedHeaders = [];
    parsedRows    = [];
  });

  function showError(msg) { errorMsg.textContent = msg; }
  function escHtml(str) {
    return String(str)
      .replace(/&/g, '&amp;')
      .replace(/</g, '&lt;')
      .replace(/>/g, '&gt;')
      .replace(/"/g, '&quot;');
  }
})();
</script>
