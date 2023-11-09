const { contextBridge, ipcRenderer } = require('electron')

contextBridge.exposeInMainWorld('electronAPI', {
  setTitle: (title) => ipcRenderer.send('set-title', title)
})


// const { contextBridge, ipcRenderer } = require('electron')

// contextBridge.exposeInMainWorld('electronAPI', {
//   openFile: () => ipcRenderer.invoke('dialog:openFile')
// })