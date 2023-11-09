const { app, BrowserWindow, ipcMain } = require('electron')
const path = require('node:path')

function createWindow () {
  const mainWindow = new BrowserWindow({
    webPreferences: {
      preload: path.join(__dirname, 'preload.js')
    }
  })

  ipcMain.on('set-title', (event, title) => {
    const webContents = event.sender
    const win = BrowserWindow.fromWebContents(webContents)
    win.setTitle(title)
  })

  mainWindow.loadFile('index.html')
}

app.whenReady().then(() => {
  createWindow()

  app.on('activate', function () {
    if (BrowserWindow.getAllWindows().length === 0) createWindow()
  })
})

app.on('window-all-closed', function () {
  if (process.platform !== 'darwin') app.quit()
})


// const { app, BrowserWindow, ipcMain, dialog } = require('electron')
// const path = require('node:path')

// async function handleFileOpen() {
//     const { canceled, filePaths } = await dialog.showOpenDialog()
//     if (!canceled) {
//         return filePaths[0]
//     }
// }

// function createWindow() {
//     const mainWindow = new BrowserWindow({
//         webPreferences: {
//             preload: path.join(__dirname, 'preload.js')
//         }
//     })
//     mainWindow.loadFile('index.html')
// }

// app.whenReady().then(() => {
//     ipcMain.handle('dialog:openFile', handleFileOpen)
//     createWindow()
//     app.on('activate', function () {
//         if (BrowserWindow.getAllWindows().length === 0) createWindow()
//     })
// })

// app.on('window-all-closed', function () {
//     if (process.platform !== 'darwin') app.quit()
// })