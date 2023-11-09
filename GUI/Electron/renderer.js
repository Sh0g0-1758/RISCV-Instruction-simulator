const setButton = document.getElementById('btn')
const titleInput = document.getElementById('title')
setButton.addEventListener('click', () => {
  const title = titleInput.value
  window.electronAPI.setTitle(title)
})

// const btn = document.getElementById('btn')
// const filePathElement = document.getElementById('filePath')

// btn.addEventListener('click', async () => {
//     const filePath = await window.electronAPI.openFile()
//     filePathElement.innerText = filePath
// })