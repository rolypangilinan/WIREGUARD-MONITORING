/**
 * AI Widget - Universal Floating Assistant for Cycle Time Monitoring
 * Self-initializing: just include this script on any page.
 * Context-aware: detects which page user is on via URL.
 */
(function() {
    'use strict';

    // State
    let chatHistory = [];
    let testResults = {};
    let healthScore = null;
    let configOpen = false;
    const pageContext = detectPageContext();

    // Detect page context from URL
    function detectPageContext() {
        const path = window.location.pathname;
        if (path.match(/\/process\d/)) return 'process' + (path.match(/\d+/) || ['1'])[0];
        if (path.includes('material_setter')) return 'material_setter';
        if (path.includes('cycle_graph')) return 'cycle_graph';
        if (path.includes('line_trend')) return 'line_trend';
        if (path.includes('manpower')) return 'manpower';
        if (path.includes('standard_time')) return 'standard_time';
        if (path.includes('settings')) return 'settings';
        return 'main_menu';
    }

    // Build the widget DOM
    function createWidget() {
        // Floating button
        const btn = document.createElement('button');
        btn.id = 'ai-widget-btn';
        btn.innerHTML = '&#129302;'; // robot emoji
        btn.title = 'AI Assistant';
        btn.onclick = togglePanel;
        document.body.appendChild(btn);

        // Chat panel
        const panel = document.createElement('div');
        panel.id = 'ai-widget-panel';
        panel.innerHTML = `
            <div id="ai-widget-header">
                <div class="ai-title">
                    <span>&#129302;</span> AI Assistant
                    <span id="ai-health-badge"></span>
                </div>
                <button class="ai-close" onclick="document.getElementById('ai-widget-panel').classList.remove('open')">&times;</button>
            </div>
            <div id="ai-widget-messages"></div>
            <div id="ai-widget-actions">
                <button onclick="window.aiWidget.diagnose()">Diagnose</button>
                <button onclick="window.aiWidget.quickFix()">Fix Issues</button>
                <button onclick="window.aiWidget.runTests()">Run Tests</button>
                <button onclick="window.aiWidget.toggleConfig()">&#9881;</button>
            </div>
            <div id="ai-config-panel">
                <label>AI Provider</label>
                <select id="ai-cfg-provider">
                    <option value="offline">Offline (Free, no API key)</option>
                    <option value="gemini">Google Gemini (Free tier)</option>
                    <option value="openai">OpenAI GPT (Paid)</option>
                </select>
                <label>API Key</label>
                <input type="password" id="ai-cfg-key" placeholder="Enter API key (optional for offline)">
                <label>Model</label>
                <input type="text" id="ai-cfg-model" placeholder="e.g. gemini-2.0-flash or gpt-3.5-turbo">
                <button class="ai-config-save" onclick="window.aiWidget.saveConfig()">Save Settings</button>
            </div>
            <div id="ai-widget-input-area">
                <input type="text" id="ai-widget-input" placeholder="Ask me anything..." onkeydown="if(event.key==='Enter')window.aiWidget.send()">
                <button id="ai-widget-send" onclick="window.aiWidget.send()">&#9654;</button>
            </div>
        `;
        document.body.appendChild(panel);

        // Load config and show welcome
        loadConfig();
        addMessage('ai', getWelcomeMessage());
    }

    function getWelcomeMessage() {
        const greetings = {
            'main_menu': "Hi! I'm your AI assistant. I can diagnose system health, fix issues, or answer questions. Try clicking 'Diagnose'!",
            'material_setter': "Hi! I'm here to help with kitting and materials. Ask me about job orders, QR printing, or material setup.",
            'cycle_graph': "Hi! I can help explain cycle time data and identify slow processes.",
            'line_trend': "Hi! I can help analyze per-kitting trends and identify outliers.",
            'manpower': "Hi! Need help with operator assignments or shift coverage?",
            'settings': "Hi! I can help with configuration settings.",
            'standard_time': "Hi! I can help with standard time setup for processes."
        };
        if (pageContext.startsWith('process')) {
            return "Hi! I'm here to help with this process. Ask about timers, NG handling, or operator issues.";
        }
        return greetings[pageContext] || greetings['main_menu'];
    }

    function togglePanel() {
        const panel = document.getElementById('ai-widget-panel');
        panel.classList.toggle('open');
    }

    // Chat functions
    function addMessage(role, text) {
        const container = document.getElementById('ai-widget-messages');
        if (!container) return;
        const div = document.createElement('div');
        div.className = 'ai-msg ' + role;
        div.innerHTML = '<div class="bubble">' + escapeHtml(text) + '</div>';
        container.appendChild(div);
        container.scrollTop = container.scrollHeight;
        chatHistory.push({role: role === 'ai' ? 'assistant' : 'user', content: text});
    }

    function addTypingIndicator() {
        const container = document.getElementById('ai-widget-messages');
        const div = document.createElement('div');
        div.className = 'ai-msg ai';
        div.id = 'ai-typing';
        div.innerHTML = '<div class="bubble"><span class="typing">Thinking...</span></div>';
        container.appendChild(div);
        container.scrollTop = container.scrollHeight;
    }

    function removeTypingIndicator() {
        const el = document.getElementById('ai-typing');
        if (el) el.remove();
    }

    function escapeHtml(text) {
        return text.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/\n/g, '<br>');
    }

    async function sendMessage() {
        const input = document.getElementById('ai-widget-input');
        const msg = input.value.trim();
        if (!msg) return;

        input.value = '';
        addMessage('user', msg);
        addTypingIndicator();

        try {
            const resp = await fetch('/api/ai_chat', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({
                    message: msg,
                    test_results: testResults,
                    history: chatHistory.slice(-10),
                    page_context: pageContext
                })
            });
            const data = await resp.json();
            removeTypingIndicator();
            if (data.success) {
                addMessage('ai', data.response);
            } else {
                addMessage('ai', 'Error: ' + (data.error || 'Unknown error'));
            }
        } catch (e) {
            removeTypingIndicator();
            addMessage('ai', 'Connection error. Is the server running?');
        }
    }

    // Diagnose function
    async function diagnose() {
        addMessage('ai', 'Running system diagnostics...');
        addTypingIndicator();

        try {
            // Run quick API tests
            const tests = [
                {id: 'b16', url: '/api/server_start_time', method: 'GET'},
                {id: 'b17', url: '/api/get_settings', method: 'GET'},
                {id: 'b18', url: '/api/get_manpower', method: 'GET'},
                {id: 'b19', url: '/api/get_standard_times', method: 'GET'},
                {id: 'b20', url: '/api/get_kitting_materials', method: 'GET'},
                {id: 'b21', url: '/api/get_model_codes', method: 'GET'},
                {id: 'b22', url: '/api/get_all_main_db', method: 'GET'},
                {id: 'b23', url: '/api/get_all_kitting_summary', method: 'GET'},
                {id: 'b25', url: '/api/get_elapsed_time_settings', method: 'GET'},
                {id: 'b30', url: '/', method: 'GET'},
            ];

            const results = {};
            for (const t of tests) {
                try {
                    const r = await fetch(t.url, {method: t.method});
                    if (r.ok) {
                        const json = await r.json().catch(() => null);
                        results[t.id] = {passed: true, detail: 'OK'};
                    } else {
                        results[t.id] = {passed: false, detail: 'HTTP ' + r.status};
                    }
                } catch (e) {
                    results[t.id] = {passed: false, detail: e.message};
                }
            }

            // Check SATO printer
            try {
                const pr = await fetch('/api/run_self_test', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/json'},
                    body: JSON.stringify({test_id: 'sato_active'})
                });
                const prd = await pr.json();
                results['b10'] = {passed: prd.success, detail: prd.detail || ''};
            } catch (e) {
                results['b10'] = {passed: false, detail: 'Could not check printer'};
            }

            testResults = results;

            // Get AI diagnosis
            const diagResp = await fetch('/api/ai_diagnose', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({results: results})
            });
            const diagData = await diagResp.json();
            removeTypingIndicator();

            if (diagData.success) {
                healthScore = diagData.health_score;
                updateHealthBadge();
                addMessage('ai', diagData.report);
            } else {
                addMessage('ai', 'Diagnostics error: ' + (diagData.error || 'Unknown'));
            }
        } catch (e) {
            removeTypingIndicator();
            addMessage('ai', 'Could not run diagnostics. Server may be offline.');
        }
    }

    // Quick Fix
    function quickFix() {
        if (Object.keys(testResults).length === 0) {
            addMessage('ai', "No test results yet. Click 'Diagnose' first to identify issues.");
            return;
        }
        // Trigger chat with fix request
        const input = document.getElementById('ai-widget-input');
        input.value = 'How to fix errors?';
        sendMessage();
    }

    // Run Tests (auto-sequence)
    async function runTests() {
        addMessage('ai', 'Starting AI auto-sequenced tests...');
        addTypingIndicator();

        try {
            const seqResp = await fetch('/api/ai_auto_sequence', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({})
            });
            const seqData = await seqResp.json();
            removeTypingIndicator();

            if (seqData.success) {
                addMessage('ai', `Test sequence: ${seqData.total} tests ordered by dependencies.\nRunning diagnostics now...`);
                diagnose();
            } else {
                addMessage('ai', 'Could not generate test sequence.');
            }
        } catch (e) {
            removeTypingIndicator();
            addMessage('ai', 'Error connecting to server.');
        }
    }

    // Config
    function toggleConfig() {
        configOpen = !configOpen;
        const panel = document.getElementById('ai-config-panel');
        if (configOpen) {
            panel.classList.add('open');
        } else {
            panel.classList.remove('open');
        }
    }

    async function loadConfig() {
        try {
            const resp = await fetch('/api/ai_config');
            const data = await resp.json();
            if (data.success && data.config) {
                const cfg = data.config;
                const provEl = document.getElementById('ai-cfg-provider');
                const keyEl = document.getElementById('ai-cfg-key');
                const modelEl = document.getElementById('ai-cfg-model');
                if (provEl) provEl.value = cfg.provider || 'offline';
                if (keyEl) keyEl.placeholder = cfg.api_key_masked || 'Enter API key';
                if (modelEl) modelEl.value = cfg.model || '';
            }
        } catch (e) { /* offline - ignore */ }
    }

    async function saveConfig() {
        const provider = document.getElementById('ai-cfg-provider').value;
        const api_key = document.getElementById('ai-cfg-key').value;
        const model = document.getElementById('ai-cfg-model').value;

        const body = {provider: provider};
        if (api_key) body.api_key = api_key;
        if (model) body.model = model;

        try {
            const resp = await fetch('/api/ai_config', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify(body)
            });
            const data = await resp.json();
            if (data.success) {
                addMessage('ai', 'Settings saved! Provider: ' + provider);
                toggleConfig();
            } else {
                addMessage('ai', 'Error saving: ' + (data.error || 'Unknown'));
            }
        } catch (e) {
            addMessage('ai', 'Could not save settings. Server offline?');
        }
    }

    // Health badge
    function updateHealthBadge() {
        const badge = document.getElementById('ai-health-badge');
        if (!badge || healthScore === null) return;
        badge.textContent = healthScore + '%';
        if (healthScore >= 80) badge.style.background = 'rgba(40,167,69,0.3)';
        else if (healthScore >= 50) badge.style.background = 'rgba(255,193,7,0.3)';
        else badge.style.background = 'rgba(220,53,69,0.3)';
    }

    // Initialize on DOM ready
    function init() {
        if (document.readyState === 'loading') {
            document.addEventListener('DOMContentLoaded', createWidget);
        } else {
            createWidget();
        }
    }

    // Public API
    window.aiWidget = {
        send: sendMessage,
        diagnose: diagnose,
        quickFix: quickFix,
        runTests: runTests,
        toggleConfig: toggleConfig,
        saveConfig: saveConfig,
        getTestResults: function() { return testResults; },
        getHealthScore: function() { return healthScore; }
    };

    init();
})();
