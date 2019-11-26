$file = './Source/Almonaster/GameEngine/GameEngine.cpp'
$content = Get-Content -Path $file
Remove-Item -Path $file

ForEach ($row in $content)
{
    if ($row.Contains('#define ALMONASTER_VERSION'))
    {
        $index = $row.IndexOf('VERSION ')
        if ($index -ne -1)
        {
            $index += 'VERSION '.Length
            $version = [version]$row.Substring($index).Trim()
 
            Add-Content -Path $file ('#define ALMONASTER_VERSION ' + '{0}.{1}.{2}' -f $version.Major, $version.Minor, ($version.Build + 1))
        }
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
        $index = $row.IndexOf('VERSION ')
        if ($index -ne -1)
        {
            $index += 'VERSION '.Length
            $version = [version]$row.Substring($index).Trim()
            $newVersion = '{0}.{1}.{2}' -f $version.Major, $version.Minor, ($version.Build + 1);

            Add-Content -Path $file ('#define ALAJAR_VERSION ' + $newVersion)
        }
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
                $version = [version]$row.Substring($startIndex, $endIndex - $startIndex).Trim()
                $newVersion = '{0}.{1}.{2}' -f $version.Major, $version.Minor, ($version.Build + 1);

                $newRow = $row.Substring(0, $startIndex) + $newVersion + $row.SubString($endIndex)
                Add-Content -Path $file $newRow
            }
        }
    }
    else
    {
        Add-Content -Path $file $row
    }
}
