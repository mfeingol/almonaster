param(
    [Parameter(Mandatory=$true)][string]$AlmonasterVersion,
    [Parameter(Mandatory=$true)][string]$AlajarVersion
    )

$file = './Source/Almonaster/GameEngine/GameEngine.cpp'
$content = Get-Content -Path $file
Remove-Item -Path $file

ForEach ($row in $content)
{
    if ($row.Contains('#define ALMONASTER_VERSION'))
    {
        Add-Content -Path $file ('#define ALMONASTER_VERSION ' + $AlmonasterVersion)
    }
    else
    {
        Add-Content -Path $file $row
    }
}

$file = './Source/AlajarDll/HttpServer.h'
$content = Get-Content -Path $file
Remove-Item -Path $file

ForEach ($row in $content)
{
    if ($row.Contains('#define ALAJAR_VERSION'))
    {
        Add-Content -Path $file ('#define ALAJAR_VERSION ' + $AlajarVersion)
    }
    else
    {
        Add-Content -Path $file $row
    }
}

$file = './.github/workflows/release.yml'
$content = Get-Content -Path $file
Remove-Item -Path $file

ForEach ($row in $content)
{
    if ($row.Contains('/almonaster-'))
    {
        $startIndex = $row.IndexOf('/almonaster-')
        if ($startIndex -ne -1)
        {
            $startIndex += '/almonaster-'.Length
            $endIndex = $row.IndexOf('-', $startIndex)
            if ($endIndex -ne -1)
            {
                $newRow = $row.Substring(0, $startIndex) + $AlmonasterVersion + $row.SubString($endIndex)
                Add-Content -Path $file $newRow
            }
        }
    }
    else
    {
        Add-Content -Path $file $row
    }
}
